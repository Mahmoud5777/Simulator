#include <gtest/gtest.h>
#include "../include/CanManager.hpp"
#include <vector>
#include <string>

// --- FakeBusManager pour simuler l'envoi et la réception sur un bus CAN ---
class FakeBusManager : public BusManager {
public:
    std::vector<FrameCAN> sentFrames;  // Stocke toutes les trames envoyées
    FrameCAN frameToReceive;            // Trame à recevoir (Flow Control ou message)

    void send(const FrameCAN &frame) override {
        sentFrames.push_back(frame);
    }

    FrameCAN receive() override {
        return frameToReceive; // retourne la trame injectée
    }
};

// --- Test unitaire : envoi d'une Single Frame ---
TEST(CanManagerTest, SendSingleFrame) {
    FakeBusManager bus;
    CanManager can(bus);

    std::string msg = "ABC";   // Petit message → SF
    can.send(msg);

    ASSERT_EQ(bus.sentFrames.size(), 1);

    auto data = bus.sentFrames[0].getData();
    ASSERT_EQ(data.size(), msg.size() + 1); // 1 octet PCI + payload

    std::string payload(data.begin() + 1, data.end());
    EXPECT_EQ(payload, msg);
}

// --- Test unitaire : envoi d'un message long (FF + CF) avec Flow Control simulé ---
TEST(CanManagerTest, SendMultiFrame) {
    FakeBusManager bus;
    CanManager can(bus);

    std::string msg;
    for (char c = 'A'; c <= 'Z'; ++c) msg.push_back(c); // 26 caractères

    // Crée une trame Flow Control simulée
    FrameCAN fakeFC;
    fakeFC.setData({0x30, 0x00, 0x00}); // 0x30 = Flow Control, 0x00 = block size, 0x00 = STmin

    // Injecte-la dans le bus simulé
    bus.frameToReceive = fakeFC;

    can.send(msg);

    ASSERT_GT(bus.sentFrames.size(), 1);

    auto firstFrame = bus.sentFrames[0].getData();
    EXPECT_EQ(firstFrame[0] >> 4, 0x1); // vérifie que c'est un First Frame

    int lengthFF = ((firstFrame[0] & 0x0F) << 8) | firstFrame[1];
    EXPECT_EQ(lengthFF, msg.size());

    // Vérifie que chaque CF contient ≤ 7 octets de données
    for (size_t i = 1; i < bus.sentFrames.size(); ++i) {
        EXPECT_LE(bus.sentFrames[i].getData().size(), 8);
    }
}

// --- Test unitaire : réception d'une trame unique ---
TEST(CanManagerTest, ReceiveMessage) {
    FakeBusManager bus;

    FrameCAN frame;
    frame.setData({'\x03','H','e','y'}); // PCI=0x03, payload="Hey"
    bus.frameToReceive = frame;

    CanManager can(bus);
    std::string msg = can.receive();

    EXPECT_EQ(msg, "Hey");
}

// --- Test unitaire : cycle complet send + receive simplifié ---
TEST(CanManagerTest, SendAndReceiveCycle) {
    FakeBusManager bus;
    CanManager can(bus);

    std::string original = "CAN TP Test";

    // Crée un Flow Control simulé
    FrameCAN fakeFC;
    fakeFC.setData({0x30, 0x00, 0x00});
    bus.frameToReceive = fakeFC;

    can.send(original);

    ASSERT_FALSE(bus.sentFrames.empty());

    // Simule la réception de la première trame envoyée
    bus.frameToReceive = bus.sentFrames[0];
    std::string received = can.receive();

    EXPECT_FALSE(received.empty());
}

// --- Main pour lancer tous les tests ---
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

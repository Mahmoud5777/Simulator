// test.cpp
#include <gtest/gtest.h>
#include "../include/CanManager.hpp"
#include <vector>
#include <string>

// --- FakeBusManager pour simuler l'envoi et la réception sur un bus CAN ---
class FakeBusManager : public BusManager {
public:
    std::vector<FrameCAN> sentFrames;  // Stocke toutes les trames envoyées
    FrameCAN frameToReceive;           // Frame simulée à recevoir

    // Redéfinition de send pour capturer les trames
    void send(const FrameCAN &frame) override {
        sentFrames.push_back(frame);
    }

    // Redéfinition de receive pour retourner une trame simulée
    FrameCAN receive() override {
        return frameToReceive;
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

    // Vérifie que la SF a 1 octet PCI + données
    ASSERT_EQ(data.size(), msg.size() + 1);

    // Vérifie que le payload correspond au message
    std::string payload(data.begin() + 1, data.end());
    EXPECT_EQ(payload, msg);
}

// --- Test unitaire : envoi d'un message long (FF + CF) ---
TEST(CanManagerTest, SendMultiFrame) {
    FakeBusManager bus;
    CanManager can(bus);

    std::string msg = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // 26 caractères → FF + CF
    can.send(msg);

    ASSERT_GT(bus.sentFrames.size(), 1);

    auto firstFrame = bus.sentFrames[0].getData();

    // Vérifie que c'est bien un First Frame (0x1x en haut)
    EXPECT_EQ(firstFrame[0] >> 4, 0x1);

    // Récupère la longueur encodée dans FF
    int len = ((firstFrame[0] & 0x0F) << 8) | firstFrame[1];
    EXPECT_EQ(len, msg.size());

    // Vérifie que les CF contiennent ≤7 octets
    for (size_t i = 1; i < bus.sentFrames.size(); ++i) {
        EXPECT_LE(bus.sentFrames[i].getData().size(), 8);
    }
}

// --- Test unitaire : réception d'une trame unique ---
TEST(CanManagerTest, ReceiveMessage) {
    FakeBusManager bus;

    // Simule la trame à recevoir (Single Frame)
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

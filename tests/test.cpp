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

    bool createVCAN() override { return true; }
    bool init() override { return true; }
    void closeSocket() override {}
};

// --- Test unitaire : envoi d'une Single Frame ---
TEST(CanManagerTest, SendSingleFrame) {
    FakeBusManager bus;
    CanManager can(bus);

    std::string msg = "ABC";            // Petit message → SF
    can.send(msg);

    ASSERT_EQ(bus.sentFrames.size(), 1);           
    EXPECT_EQ(bus.sentFrames[0].getData().size(), msg.size()); // utilisation de getData()
}

// --- Test unitaire : envoi d'un message long (First Frame + Consecutive Frames) ---
TEST(CanManagerTest, SendMultiFrame) {
    FakeBusManager bus;
    CanManager can(bus);

    std::string msg = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // Long message → FF + CF
    can.send(msg);

    ASSERT_GT(bus.sentFrames.size(), 1);
    EXPECT_EQ(bus.sentFrames[0].getData()[0], msg.size()); // First Frame contient la taille

    for (size_t i = 1; i < bus.sentFrames.size(); ++i) {
        EXPECT_LE(bus.sentFrames[i].getData().size(), 7);  // Chaque CF ≤7 octets
    }
}

// --- Test unitaire : réception d'une trame unique ---
TEST(CanManagerTest, ReceiveMessage) {
    FakeBusManager bus;

    // Simule la trame à recevoir
    FrameCAN frame;
    frame.setData({'H','e','l','l','o'});  // utilisation de setData()
    bus.frameToReceive = frame;

    CanManager can(bus);
    std::string msg = can.receive();

    EXPECT_EQ(msg, "Hello");
}

// --- Test unitaire : cycle complet send + receive simplifié ---
TEST(CanManagerTest, SendAndReceiveCycle) {
    FakeBusManager bus;
    CanManager can(bus);

    std::string original = "CAN TP Test";
    can.send(original);

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

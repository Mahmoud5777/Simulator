// test.cpp
#include <gtest/gtest.h>
#include "../include/CanManager.hpp"
#include <vector>
#include <string>

// --- FakeBusManager pour simuler l'envoi et la réception sur un bus CAN ---
class FakeBusManager : public BusManager {
public:
    std::vector<FrameCAN> sentFrames;  // Stocke toutes les trames envoyées
    FrameCAN frameToReceive;            // Frame simulée à recevoir

    // Redéfinition de send pour capturer les trames au lieu de les envoyer sur le socket
    void send(const FrameCAN &frame) override {
        sentFrames.push_back(frame);
    }

    // Redéfinition de receive pour retourner une trame simulée
    FrameCAN receive() override {
        return frameToReceive;
    }

    // Les autres méthodes (createVCAN, init, closeSocket) peuvent rester vides pour le test
    bool createVCAN() override { return true; }
    bool init() override { return true; }
    void closeSocket() override {}
};

// --- Test unitaire : envoi d'une Single Frame ---
TEST(CanManagerTest, SendSingleFrame) {
    FakeBusManager bus;
    CanManager can(bus);

    std::string msg = "ABC";            // Petit message → SF
    can.send(msg);                      // Appel de la méthode send

    ASSERT_EQ(bus.sentFrames.size(), 1);           // Vérifie qu'une seule trame a été envoyée
    EXPECT_EQ(bus.sentFrames[0].data.size(), msg.size()); // Vérifie la taille de la trame
}

// --- Test unitaire : envoi d'un message long (First Frame + Consecutive Frames) ---
TEST(CanManagerTest, SendMultiFrame) {
    FakeBusManager bus;
    CanManager can(bus);

    std::string msg = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // Long message → FF + CF
    can.send(msg);

    ASSERT_GT(bus.sentFrames.size(), 1);            // Vérifie qu'il y a plusieurs trames
    EXPECT_EQ(bus.sentFrames[0].data[0], msg.size()); // First Frame contient la taille totale

    for (size_t i = 1; i < bus.sentFrames.size(); ++i) {
        EXPECT_LE(bus.sentFrames[i].data.size(), 7);  // Chaque Consecutive Frame ≤7 octets
    }
}

// --- Test unitaire : réception d'une trame unique ---
TEST(CanManagerTest, ReceiveMessage) {
    FakeBusManager bus;
    // Simule la trame à recevoir
    FrameCAN frame;
    frame.data = {'H','e','l','l','o'};
    bus.frameToReceive = frame;

    CanManager can(bus);
    std::string msg = can.receive();               // Appel de receive

    EXPECT_EQ(msg, "Hello");                       // Vérifie que le message décodé est correct
}

// --- Test unitaire : cycle complet send + receive simplifié ---
TEST(CanManagerTest, SendAndReceiveCycle) {
    FakeBusManager bus;
    CanManager can(bus);

    std::string original = "CAN TP Test";
    can.send(original);                            // Envoi d'un message

    // Simule la réception de la première trame envoyée
    bus.frameToReceive = bus.sentFrames[0];
    std::string received = can.receive();

    EXPECT_FALSE(received.empty());                // Vérifie que le message reçu n’est pas vide
}

// --- Main pour lancer tous les tests ---
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);       // Initialise Google Test
    return RUN_ALL_TESTS();                        // Exécute tous les tests
}

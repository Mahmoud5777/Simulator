#include <gtest/gtest.h>
#include "../include/CanManager.hpp"
#include <vector>
#include <string>
#include <queue>

class FakeBusManager : public BusManager {
public:
    std::vector<FrameCAN> sentFrames;
    std::queue<FrameCAN> framesToReceive; // File d'attente pour les trames à recevoir
    bool autoRespondWithFlowControl = false;
    int consecutiveFramesSent = 0;

    void send(const FrameCAN &frame) override {
        sentFrames.push_back(frame);
        
        if (autoRespondWithFlowControl) {
            uint8_t pci = frame.getData()[0];
            if ((pci >> 4) == 0x2) { // C'est une Consecutive Frame
                consecutiveFramesSent++;
                if (consecutiveFramesSent % 8 == 0) {
                    // Ajoute un Flow Control automatique si configuré
                    FrameCAN fc;
                    fc.setData({0x30, 0x08, 0x00}); // BlockSize = 8, STmin = 0
                    framesToReceive.push(fc);
                }
            }
        }
    }

    FrameCAN receive() override {
        if (framesToReceive.empty()) {
            throw std::runtime_error("No frame available to receive");
        }
        FrameCAN frame = framesToReceive.front();
        framesToReceive.pop();
        return frame;
    }

    // Méthode helper pour injecter des trames
    void injectFrame(const FrameCAN &frame) {
        framesToReceive.push(frame);
    }
};

TEST(CanManagerTest, ReceiveSingleFrameWithoutFlowControl) {
    FakeBusManager bus;
    CanManager can(bus);

    // Désactive les réponses automatiques pour ce test
    bus.autoRespondWithFlowControl = false;

    // Injecte une Single Frame directement
    FrameCAN sf;
    sf.setData({0x03, 'A', 'B', 'C'});
    bus.injectFrame(sf);

    std::string received = can.receive();
    EXPECT_EQ(received, "ABC");
}

TEST(CanManagerTest, ReceiveMultiFrameWithManualFlowControl) {
    FakeBusManager bus;
    CanManager can(bus);

    bus.autoRespondWithFlowControl = false; // On contrôle manuellement le flow

    // 1. Injecte la First Frame
    FrameCAN ff;
    ff.setData({0x10, 0x0A, 'H', 'e', 'l', 'l', 'o', 'W', 'o'}); // Longueur 10
    bus.injectFrame(ff);

    // Le CanManager devrait maintenant attendre un Flow Control
    ASSERT_EQ(bus.sentFrames.size(), 0); // Pas encore de réponse

    // 2. Injecte le Flow Control manuellement
    FrameCAN fc;
    fc.setData({0x30, 0x08, 0x00});
    bus.injectFrame(fc);

    // 3. Injecte les Consecutive Frames
    FrameCAN cf1;
    cf1.setData({0x21, 'r', 'l', 'd', '!', 0x00, 0x00, 0x00}); // Séquence 1
    bus.injectFrame(cf1);

    std::string result = can.receive();
    EXPECT_TRUE(result.empty()); // Le message n'est pas encore complet

    // Pas besoin de nouveau Flow Control car < 8 CF

    FrameCAN cf2;
    cf2.setData({0x22, 0x00, 0x00}); // Séquence 2 (vide)
    bus.injectFrame(cf2);

    result = can.receive();
    EXPECT_EQ(result, "HelloWorld!");
}

TEST(CanManagerTest, ReceiveWithAutomaticFlowControl) {
    FakeBusManager bus;
    CanManager can(bus);

    bus.autoRespondWithFlowControl = true; // Active les réponses automatiques

    // 1. Injecte la First Frame
    FrameCAN ff;
    ff.setData({0x10, 0x20, 
                '1', '2', '3', '4', '5', '6', '7'}); // Longueur 32
    bus.injectFrame(ff);

    // 2. Le CanManager enverra un Flow Control automatique
    // (géré par la file d'attente interne)

    // 3. Injecte 8 Consecutive Frames
    for (int i = 0; i < 8; i++) {
        FrameCAN cf;
        std::vector<uint8_t> data = {static_cast<uint8_t>(0x20 | (i+1))};
        data.resize(8, 'A' + i);
        cf.setData(data);
        bus.injectFrame(cf);
    }

    // Le FakeBusManager devrait avoir ajouté automatiquement un Flow Control
    ASSERT_FALSE(bus.framesToReceive.empty());

    // Continuer avec d'autres trames si nécessaire...
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include <gtest/gtest.h>
#include "../include/CanManager.hpp"
#include "../include/FrameCanTP.hpp"
#include <vector>
#include <string>
#include <queue>

class FakeBusManager : public BusManager {
public:
    std::vector<FrameCAN> sentFrames;
    std::queue<FrameCAN> framesToReceive;
    bool autoFlowControl = true;
    uint8_t blockSize = 8;
    uint8_t separationTime = 10;

    void send(const FrameCAN &frame) override {
        sentFrames.push_back(frame);
    }

    FrameCAN receive() override {
        if (framesToReceive.empty()) {
            return FrameCAN(ID::buildSmartID(), {}); // Retourne une frame vide
        }
        
        FrameCAN frame = framesToReceive.front();
        framesToReceive.pop();
        
        // Si autoFlowControl est activé et qu'on reçoit une First Frame
        if (autoFlowControl && !frame.getData().empty() && 
            ((frame.getData()[0] & 0xF0) == 0x10)) {
            // Injecte un Flow Control automatique
            FrameCanTP ftp;
            auto fcData = ftp.CreateFlowControlFrame(0x00, blockSize, separationTime);
            framesToReceive.push(FrameCAN(frame.getFrameID(), fcData)); // Correction ici: getId() au lieu de getFrameID
        }
        
        return frame;
    }

    void injectFrame(const FrameCAN &frame) {
        framesToReceive.push(frame);
    }
};


TEST(CanManagerTest, SendSingleFrame) {
    FakeBusManager bus;
    CanManager can(bus);

    std::string msg = "Hello";
    can.send(msg);

    ASSERT_EQ(bus.sentFrames.size(), 1);
    
    auto frameData = bus.sentFrames[0].getData();
    EXPECT_EQ(frameData.size(), msg.size() + 1); // PCI + données
    
    // Vérifie que c'est bien une Single Frame
    EXPECT_EQ((frameData[0] & 0xF0), 0x00);
    
    // Vérifie la longueur
    EXPECT_EQ((frameData[0] & 0x0F), msg.size());
    
    // Vérifie le payload
    std::string payload(frameData.begin() + 1, frameData.end());
    EXPECT_EQ(payload, msg);
}

TEST(CanManagerTest, SendMultiFrame) {
    FakeBusManager bus;
    CanManager can(bus);

    // Désactive les Flow Control automatiques pour contrôler manuellement
    bus.autoFlowControl = false;
    
    // Crée un message long (plus de 7 caractères)
    std::string msg(50, 'X'); // 50 caractères 'X'
    
    // Injecte un Flow Control manuel
    FrameCanTP ftp;
    auto fcData = ftp.CreateFlowControlFrame(0x00, bus.blockSize, bus.separationTime);
    bus.injectFrame(FrameCAN(ID::buildSmartID(), fcData));
    
    can.send(msg);
    
    // Doit avoir au moins 1 First Frame + plusieurs Consecutive Frames
    ASSERT_GT(bus.sentFrames.size(), 1);
    
    // Vérifie la First Frame
    auto ffData = bus.sentFrames[0].getData();
    EXPECT_EQ((ffData[0] & 0xF0), 0x10);
    
    // Vérifie la longueur totale
    uint16_t totalLength = ((ffData[0] & 0x0F) << 8 | ffData[1]);
    EXPECT_EQ(totalLength, msg.size());
}

TEST(CanManagerTest, ReceiveSingleFrame) {
    FakeBusManager bus;
    CanManager can(bus);

    // Prépare une Single Frame
    FrameCanTP ftp;
    std::string msg = "Test123";
    auto sfData = ftp.CreateSingleFrame(std::vector<uint8_t>(msg.begin(), msg.end()), msg.size());
    
    // Injecte la frame
    bus.injectFrame(FrameCAN(ID::buildSmartID(), sfData));
    
    std::string received = can.receive();
    EXPECT_EQ(received, msg);
}

TEST(CanManagerTest, ReceiveMultiFrame) {
    FakeBusManager bus;
    CanManager can(bus);

    // Désactive les Flow Control automatiques
    bus.autoFlowControl = false;
    
    // Crée un message long
    std::string msg = "This is a long message that requires multiple frames";
    
    // Prépare les trames
    FrameCanTP ftp;
    
    // First Frame (contient les 6 premiers caractères + longueur totale)
    std::vector<uint8_t> ffPayload(msg.begin(), msg.begin() + 6);
    auto ffData = ftp.CreateFirstFrame(ffPayload, msg.size());
    bus.injectFrame(FrameCAN(ID::buildSmartID(), ffData));
    
    // Injecte le Flow Control (Continue, BlockSize=8, STmin=10ms)
    auto fcData = ftp.CreateFlowControlFrame(0x00, 8, 10);
    bus.injectFrame(FrameCAN(ID::buildSmartID(), fcData));
    
    // Consecutive Frames (le reste du message)
    size_t offset = 6;
    uint8_t seqNum = 1;
    while (offset < msg.size()) {
        size_t chunkSize = std::min<size_t>(7, msg.size() - offset);
        auto cfData = ftp.CreateConsecutiveFrame(
            std::vector<uint8_t>(msg.begin() + offset, msg.begin() + offset + chunkSize),
            seqNum++
        );
        bus.injectFrame(FrameCAN(ID::buildSmartID(), cfData));
        offset += chunkSize;
    }
    
    std::string received = can.receive();
    
    // Vérifications
    EXPECT_EQ(received.size(), msg.size());
    EXPECT_EQ(received, msg);
    
    // Vérifie qu'aucun octet de contrôle ne s'est glissé dans les données
    for (size_t i = 0; i < received.size(); ++i) {
        EXPECT_EQ(received[i], msg[i]) 
            << "Différence à la position " << i << ": attendu '" << msg[i] 
            << "' (0x" << std::hex << (int)msg[i] << "), reçu '" << received[i] 
            << "' (0x" << (int)received[i] << ")" << std::dec;
    }
}

TEST(CanManagerTest, ReceiveWithFlowControl) {
    FakeBusManager bus;
    CanManager can(bus);

    // Active les Flow Control automatiques
    bus.autoFlowControl = true;
    
    std::string msg = "Another long message for flow control testing";
    
    // Injecte juste la First Frame, le reste sera géré automatiquement
    FrameCanTP ftp;
    auto ffData = ftp.CreateFirstFrame(std::vector<uint8_t>(msg.begin(), msg.begin()+6), msg.size());
    bus.injectFrame(FrameCAN(ID::buildSmartID(), ffData));
    
    // Injecte les Consecutive Frames
    size_t offset = 6;
    uint8_t seqNum = 1;
    while (offset < msg.size()) {
        size_t chunkSize = std::min<size_t>(7, msg.size() - offset);
        auto cfData = ftp.CreateConsecutiveFrame(
            std::vector<uint8_t>(msg.begin()+offset, msg.begin()+offset+chunkSize),
            seqNum++
        );
        bus.injectFrame(FrameCAN(ID::buildSmartID(), cfData));
        offset += chunkSize;
    }
    
    std::string received = can.receive();
    EXPECT_EQ(received, msg);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
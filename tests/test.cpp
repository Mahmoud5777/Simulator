
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

struct CoutSilencer {
    std::streambuf* oldCout;
    std::ostringstream oss;
    CoutSilencer() { oldCout = std::cout.rdbuf(oss.rdbuf()); }
    ~CoutSilencer() { std::cout.rdbuf(oldCout); }
};



TEST(CanManagerTest, SendSingleFrame) {
    CoutSilencer silence;
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
    CoutSilencer silence;
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
    CoutSilencer silence;
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
    CoutSilencer silence;
    FakeBusManager bus;
    CanManager can(bus);

    bus.autoFlowControl = true; // Laisse le fake générer les FC automatiquement

    std::string msg = "This is a long message that requires multiple frames";

    // Injecte seulement la First Frame
    FrameCanTP ftp;
    auto ffData = ftp.CreateFirstFrame(
        std::vector<uint8_t>(msg.begin(), msg.begin()+6),
        msg.size()
    );
    bus.injectFrame(FrameCAN(ID::buildSmartID(), ffData));

    // Les Consecutive Frames seront injectées par le fake après chaque Flow Control
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


TEST(CanManagerTest, ReceiveWithFlowControl) {
    CoutSilencer silence;
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

TEST(CanManagerTest, SendWithFlowControlStop) {
    CoutSilencer silence;
    FakeBusManager bus;
    CanManager can(bus);

    bus.autoFlowControl = false; // On contrôle manuellement

    std::string msg(20, 'A'); // message long > 7 octets

    FrameCanTP ftp;

    // Injecte la première Flow Control : STOP
    auto fcStopData = ftp.CreateFlowControlFrame(0x31, 0, 0); // FC=Stop
    bus.injectFrame(FrameCAN(ID::buildSmartID(), fcStopData));

    // Ensuite injecte une Flow Control continue pour permettre de reprendre
    auto fcContinueData = ftp.CreateFlowControlFrame(0x30, 8, 0);
    bus.injectFrame(FrameCAN(ID::buildSmartID(), fcContinueData));

    // Envoi du message
    can.send(msg);

    // Vérifie que toutes les trames ont été envoyées correctement
    ASSERT_GT(bus.sentFrames.size(), 1);

    // Vérifie que le premier bloc a bien été envoyé avant le STOP
    EXPECT_EQ(bus.sentFrames[0].getData()[0] >> 4, 0x1); // First Frame
}

TEST(CanManagerTest, SendWithFlowControlAbort) {
    CoutSilencer silence;
    FakeBusManager bus;
    CanManager can(bus);

    bus.autoFlowControl = false; // On contrôle manuellement
    std::string msg(20, 'E');    // Message long > 7 octets
    FrameCanTP ftp;

    // Crée une Flow Control d’erreur (Abort)
    auto fcAbortData = ftp.CreateFlowControlFrame(0x32, 0, 0); // 0x32 = Abort/Error
    bus.injectFrame(FrameCAN(ID::buildSmartID(), fcAbortData));

    // Capture std::cout avant d’envoyer
    std::ostringstream oss;
    std::streambuf* oldCout = std::cout.rdbuf(oss.rdbuf());

    // Envoie du message (une seule fois)
    can.send(msg);

    // Restaure std::cout
    std::cout.rdbuf(oldCout);
    std::string output = oss.str();

    // Vérifie que seulement la First Frame a été envoyée
    ASSERT_EQ(bus.sentFrames.size(), 1);

    // Vérifie que c’est bien une First Frame
    auto ffData = bus.sentFrames[0].getData();
    EXPECT_EQ((ffData[0] & 0xF0), 0x10);

    // Vérifie que le message d’abort a été affiché
    EXPECT_NE(output.find("Transmission aborted !!"), std::string::npos);
}





int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
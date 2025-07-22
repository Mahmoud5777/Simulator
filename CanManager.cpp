#include "CanManager.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include "FrameCanTP.hpp"
#include "BusManager.hpp"
#include <thread>

CanManager::CanManager() {}
CanManager::~CanManager() {}

std::vector<uint8_t> CanManager::encoder(const std::string & data) {
    std::vector<uint8_t> encodedData(data.begin(), data.end());
    return encodedData;
}

std::string CanManager::decoder(const std::vector<uint8_t> & data) {
    return std::string(data.begin(), data.end());
}

void CanManager::send(const std::string &frame){
    std::vector<uint8_t> encodedData = encoder(frame);
    auto Data_size = encodedData.size();
    FrameCanTP frameCanTP;
    BusManager busManager;
    if (Data_size <= 7){
        std::vector<uint8_t>SingleFame=frameCanTP.CreateSingleFrame(encodedData,Data_size);
        FrameCAN canFrame(0, SingleFame);
        busManager.send(canFrame);
    } else {
        std::vector<uint8_t> DataFirstFrame(encodedData.begin(), encodedData.begin() + 6);
        encodedData.erase(encodedData.begin(), encodedData.begin() + 6);
        std::vector<uint8_t> firstFrame = frameCanTP.CreateFirstFrame(DataFirstFrame,Data_size);
        FrameCAN canFrame(0, firstFrame);
        busManager.send(canFrame);
        while ((!encodedData.empty())) {
            FrameCAN receivedFrame = busManager.receive();
            std::vector<uint8_t> FlowControlFrame = receivedFrame.getData();
            uint8_t flowStatus = FlowControlFrame[0] & 0x0F;
            uint8_t blockSize = FlowControlFrame[1];  
            uint8_t separationTime = FlowControlFrame[2];
            if (flowStatus == 0x00) { // Continue sending
                for (uint8_t i = 0; i < blockSize && !encodedData.empty(); ++i) {
                    uint8_t sequenceNumber = i + 1;
                    size_t Size = std::min<size_t>(7, encodedData.size());
                    std::vector<uint8_t> DataConseucutiveFrame(encodedData.begin(), encodedData.begin() + Size);
                    std::vector<uint8_t> consecutiveFrame = frameCanTP.CreateConsecutiveFrame(DataConseucutiveFrame, sequenceNumber);
                    FrameCAN canFrame(0, consecutiveFrame);
                    busManager.send(canFrame);
                    encodedData.erase(encodedData.begin(), encodedData.begin() + 7); 
                }
            } else if (flowStatus == 0x01) { // Wait
                std::this_thread::sleep_for(std::chrono::milliseconds(separationTime));
            } else if (flowStatus == 0x02) { // Abort
                std::cout << "Transmission aborted." << std::endl;
                break;
            }
        }
    }
}

std::string CanManager::receive() {
    BusManager bus;
    FrameCanTP frameCanTP;
    std::vector<uint8_t> Buffer;
    uint16_t expectedSize = 0;
    FrameCAN receivedFrame = bus.receive();
    std::vector<uint8_t> data = receivedFrame.getData();
    uint8_t frameType = data[0] & 0xF0;
    if (frameType == 0x00) { // Single Frame
        Buffer = frameCanTP.GetDataFromSingleFrame(data);
        // Message complet reçu en une frame, traitement ici
    } else if (frameType == 0x10) { // First Frame
        expectedSize = ((data[0] & 0x0F) << 8) | data[1]; // Taille totale message
        Buffer = frameCanTP.GetDataFromFirstFrame(data);
        // Envoyer Flow Control : Continue, BlockSize=8, SeparationTime=10ms
        std::vector<uint8_t> flowControlData = frameCanTP.CreateFlowControlFrame(0x00, 8, 10);
        FrameCAN flowControlFrame(0, flowControlData);
        bus.send(flowControlFrame);
        uint8_t framesReceivedInBlock = 0;
        const uint8_t blockSize = 8;
        const uint8_t separationTimeMs = 10;
        // Réception des Consecutive Frames
        while (Buffer.size() < expectedSize) {
            receivedFrame = bus.receive();
            data = receivedFrame.getData();
            uint8_t seqNum = data[0] & 0x0F;
            std::vector<uint8_t> consecutiveData = frameCanTP.GetDataFromConsecutiveFrame(data);
            Buffer.insert(Buffer.end(), consecutiveData.begin(), consecutiveData.end());
            framesReceivedInBlock++;
            if (blockSize != 0 && framesReceivedInBlock >= blockSize) {
                framesReceivedInBlock = 0;
                std::vector<uint8_t> fcData = frameCanTP.CreateFlowControlFrame(0x00, blockSize, separationTimeMs);
                FrameCAN fcFrame(0, fcData);
                bus.send(fcFrame);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(separationTimeMs));
        }
        // Message complet reçu
        Buffer.resize(expectedSize); // Enlever les éventuels octets excédentaires
    } else {
        std::cerr << "Type de trame non supporté ou inattendu" << std::endl;
    }
    std::string receivedData = decoder(Buffer);
    std::cout << "Données reçues: " << receivedData << std::endl;
    return receivedData;
}



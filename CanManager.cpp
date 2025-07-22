#include "CanManager.hpp"
#include "FrameCanTP.hpp"
#include <iostream>
#include <thread>
#include <algorithm>

CanManager::CanManager(BusManager& bus)
    : busManager(bus) {}

std::vector<uint8_t> CanManager::encoder(const std::string& data) {
    return std::vector<uint8_t>(data.begin(), data.end());
}

std::string CanManager::decoder(const std::vector<uint8_t>& data) {
    return std::string(data.begin(), data.end());
}

void CanManager::send(const std::string& frame) {
    std::vector<uint8_t> encodedData = encoder(frame);
    size_t dataSize = encodedData.size();

    FrameCanTP frameCanTP;

    if (dataSize <= 7) {
        auto singleFrame = frameCanTP.CreateSingleFrame(encodedData, dataSize);
        FrameCAN canFrame(0x123, singleFrame);  // Mettre un ID CAN adapté
        busManager.send(canFrame);
    } else {
        std::vector<uint8_t> dataFirstFrame(encodedData.begin(), encodedData.begin() + 6);
        encodedData.erase(encodedData.begin(), encodedData.begin() + 6);
        auto firstFrame = frameCanTP.CreateFirstFrame(dataFirstFrame, dataSize);
        FrameCAN canFrame(0x123, firstFrame);
        busManager.send(canFrame);

        while (!encodedData.empty()) {
            FrameCAN flowControlFrame = busManager.receive();
            auto flowControlData = flowControlFrame.getData();

            if (flowControlData.size() < 3) {
                std::cerr << "Flow control frame trop courte" << std::endl;
                break;
            }

            uint8_t flowStatus = flowControlData[0] & 0x0F;
            uint8_t blockSize = flowControlData[1];
            uint8_t separationTime = flowControlData[2];

            if (flowStatus == 0x00) { // Continue sending
                for (uint8_t i = 0; i < blockSize && !encodedData.empty(); ++i) {
                    size_t sizeToSend = std::min<size_t>(7, encodedData.size());
                    std::vector<uint8_t> consecutiveData(encodedData.begin(), encodedData.begin() + sizeToSend);
                    auto consecutiveFrame = frameCanTP.CreateConsecutiveFrame(consecutiveData, i + 1);
                    FrameCAN canFrame(0x123, consecutiveFrame);
                    busManager.send(canFrame);
                    encodedData.erase(encodedData.begin(), encodedData.begin() + sizeToSend);
                    std::this_thread::sleep_for(std::chrono::milliseconds(separationTime));
                }
            } else if (flowStatus == 0x01) { // Wait
                std::this_thread::sleep_for(std::chrono::milliseconds(separationTime));
            } else if (flowStatus == 0x02) { // Abort
                std::cout << "Transmission aborted." << std::endl;
                break;
            } else {
                std::cerr << "Flow control status inconnu: " << (int)flowStatus << std::endl;
                break;
            }
        }
    }
}

std::string CanManager::receive() {
    FrameCanTP frameCanTP;
    std::vector<uint8_t> buffer;
    uint16_t expectedSize = 0;

    FrameCAN receivedFrame = busManager.receive();
    if (receivedFrame.getData().empty()) {
        std::cerr << "Aucune donnée reçue" << std::endl;
        return "";
    }

    std::vector<uint8_t> data = receivedFrame.getData();
    uint8_t frameType = data[0] & 0xF0;

    if (frameType == 0x00) { // Single Frame
        buffer = frameCanTP.GetDataFromSingleFrame(data);
    } else if (frameType == 0x10) { // First Frame
        expectedSize = ((data[0] & 0x0F) << 8) | data[1];
        buffer = frameCanTP.GetDataFromFirstFrame(data);

        // Envoyer Flow Control: Continue, BlockSize=8, SeparationTime=10ms
        std::vector<uint8_t> flowControlData = frameCanTP.CreateFlowControlFrame(0x00, 8, 10);
        FrameCAN flowControlFrame(0x123, flowControlData);
        busManager.send(flowControlFrame);

        uint8_t framesReceivedInBlock = 0;
        const uint8_t blockSize = 8;
        const uint8_t separationTimeMs = 10;

        while (buffer.size() < expectedSize) {
            receivedFrame = busManager.receive();
            data = receivedFrame.getData();

            if (data.empty()) {
                std::cerr << "Trame vide reçue pendant reception consecutive" << std::endl;
                break;
            }

            std::vector<uint8_t> consecutiveData = frameCanTP.GetDataFromConsecutiveFrame(data);
            buffer.insert(buffer.end(), consecutiveData.begin(), consecutiveData.end());
            framesReceivedInBlock++;

            if (blockSize != 0 && framesReceivedInBlock >= blockSize) {
                framesReceivedInBlock = 0;
                std::vector<uint8_t> fcData = frameCanTP.CreateFlowControlFrame(0x00, blockSize, separationTimeMs);
                FrameCAN fcFrame(0x123, fcData);
                busManager.send(fcFrame);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(separationTimeMs));
        }

        if (buffer.size() > expectedSize) {
            buffer.resize(expectedSize); // couper les octets excédentaires
        }

    } else {
        std::cerr << "Type de trame non supporté ou inattendu: 0x" << std::hex << (int)frameType << std::dec << std::endl;
    }

    std::string receivedData = decoder(buffer);
    std::cout << "Données reçues: " << receivedData << std::endl;
    return receivedData;
}

#include "CanManager.hpp"
#include "FrameCanTP.hpp"
#include <iostream>
#include <thread>
#include <algorithm>
#include "BusManager.hpp"
#include "ID.hpp"

std::vector<uint8_t> CanManager::encoder(const std::string& data) {
    return std::vector<uint8_t>(data.begin(), data.end());
}

std::string CanManager::decoder(const std::vector<uint8_t>& data) {
    return std::string(data.begin(), data.end());
}

void CanManager::send(const std::string& frame) {
    ID frame_id = ID::buildSmartID(); // Génération d'un ID CAN aléatoire
    std::vector<uint8_t> encodedData = encoder(frame);
    std::cout << "Encoded data to send : { ";
    for (auto byte : encodedData) {
        std::cout << static_cast<int>(byte); // Affichage forcement en int 
        std::cout << " ";
    }
    std::cout << " } "<<std::endl;
    size_t dataSize = encodedData.size();
    if (dataSize == 0) {    
            std::cerr << "No data to send !!" << std::endl;
            return;
    }
    FrameCanTP frameCanTP;
    if (dataSize <= 7) {
        std::cout << "Sending single frame : { " ;
        auto singleFrame = frameCanTP.CreateSingleFrame(encodedData, dataSize);
        for (auto byte : singleFrame) {
            std::cout << static_cast<int>(byte) << " ";
        }
        std::cout << " } "<<std::endl;
        FrameCAN canFrame( frame_id, singleFrame);  // Mettre un ID CAN adapté
        busManager.send(canFrame);
    } else {
        std::vector<uint8_t> dataFirstFrame(encodedData.begin(), encodedData.begin() + 6);
        std::cout << "Sending first frame : { ";
        encodedData.erase(encodedData.begin(), encodedData.begin() + 6);
        auto firstFrame = frameCanTP.CreateFirstFrame(dataFirstFrame, dataSize);
        for (auto byte : firstFrame) {
            std::cout << static_cast<int>(byte) << " ";
        }
        std::cout << " } "<<std::endl;
        FrameCAN canFrame(frame_id, firstFrame);
        busManager.send(canFrame);
        while (!encodedData.empty()) {
            FrameCAN flowControlFrame = busManager.receive();
            std::cout << "Received flow control frame : { ";
            auto flowControlData = flowControlFrame.getData();
            for (auto byte : flowControlData) {
                std::cout << static_cast<int>(byte) << " ";
            }
            std::cout << " } "<<std::endl;
            uint8_t flowStatus = flowControlData[0] & 0x0F;
            uint8_t blockSize = flowControlData[1];
            uint8_t separationTime = flowControlData[2];
            std::cout << "( Flow control status: " << static_cast<int>(flowStatus) << ", Block size: " 
                      << static_cast<int>(blockSize) << ", Separation time: " 
                      << static_cast<int>(separationTime) << " ms )" << std::endl;
            if (flowStatus == 0x00) { // Continue sending
                for (uint8_t i = 0; i < blockSize && !encodedData.empty(); ++i) {
                    size_t sizeToSend = std::min<size_t>(7, encodedData.size());
                    std::vector<uint8_t> consecutiveData(encodedData.begin(), encodedData.begin() + sizeToSend);
                    std::cout << "Sending consecutive frame : { ";
                    auto consecutiveFrame = frameCanTP.CreateConsecutiveFrame(consecutiveData, i + 1);
                    for (auto byte : consecutiveFrame) {
                        std::cout << static_cast<int>(byte) << " ";
                    }
                    std::cout << " } "<<std::endl;
                    FrameCAN canFrame(frame_id, consecutiveFrame);
                    busManager.send(canFrame);
                    encodedData.erase(encodedData.begin(), encodedData.begin() + sizeToSend);
                    std::this_thread::sleep_for(std::chrono::milliseconds(separationTime));
                }
            } else if (flowStatus == 0x01) { // Wait
                std::this_thread::sleep_for(std::chrono::milliseconds(separationTime));
            } else { // Abort
                std::cout << "Transmission aborted !!" << std::endl;
                break;
            } 
        }
    }
}

std::string CanManager::receive() {
    ID frame_id = ID::buildSmartID(); // Génération d'un ID CAN aléatoire
    FrameCanTP frameCanTP;
    std::vector<uint8_t> buffer;
    uint16_t expectedSize = 0;
    FrameCAN receivedFrame = busManager.receive();
    if (receivedFrame.getData().empty()) {
        std::cerr << "No data to receive !!" << std::endl;
        return "";
    }
    std::vector<uint8_t> data = receivedFrame.getData();
    uint8_t frameType = data[0] & 0xF0;
    if (frameType == 0x00) { // Single Frame
        std::cout << "Received single frame : { ";
        for (auto byte : data) {
                std::cout << static_cast<int>(byte) << " ";
        }
        std::cout << " } "<<std::endl;
        buffer = frameCanTP.GetDataFromSingleFrame(data);
    } else if (frameType == 0x10) { // First Frame
        expectedSize = ((data[0] & 0x0F) << 8) | data[1];
        std::cout << "Received first frame : { ";
        for (auto byte : data) {
                std::cout << static_cast<int>(byte) << " ";
        }
        std::cout << " } "<<std::endl;
        buffer = frameCanTP.GetDataFromFirstFrame(data);
        // Envoyer Flow Control: Continue, BlockSize=8, SeparationTime=10ms
        std::vector<uint8_t> flowControlData = frameCanTP.CreateFlowControlFrame(0x00, 8, 10);
        std::cout << "Sending flow control frame : { ";
        for (auto byte : flowControlData) {
            std::cout << static_cast<int>(byte) << " ";
        }
        std::cout << " } "<<std::endl;
        FrameCAN flowControlFrame(frame_id, flowControlData);
        busManager.send(flowControlFrame);
        uint8_t framesReceivedInBlock = 0;
        const uint8_t blockSize = 8;
        const uint8_t separationTimeMs = 10;
        int waitCount = 0;
        const int MAX_WAIT_RETRIES = 3;
        while (buffer.size() < expectedSize) {
            receivedFrame = busManager.receive();
            data = receivedFrame.getData();
            if (data.empty()) {
                if (++waitCount > MAX_WAIT_RETRIES) {
                    std::vector<uint8_t> abortFrame = frameCanTP.CreateFlowControlFrame(0x02, 0, 0);
                    std::cout << "Sending flow control frame : { ";
                    for (auto byte : abortFrame) {
                        std::cout << static_cast<int>(byte) << " ";
                    }
                    std::cout << " } "<<std::endl;
                    busManager.send(FrameCAN(frame_id, abortFrame));
                    break; // Sortir de la boucle si trop d'attentes
                }
                std::vector<uint8_t> waitFrame = frameCanTP.CreateFlowControlFrame(0x01, 0, separationTimeMs);
                std::cout << "Sending flow control frame : { ";
                for (auto byte : waitFrame) {
                    std::cout << static_cast<int>(byte) << " ";
                }
                std::cout << " } "<<std::endl;
                busManager.send(FrameCAN(frame_id, waitFrame));
                std::this_thread::sleep_for(std::chrono::milliseconds(separationTimeMs));
                continue; // Retour au début de la boucle pour nouvelle réception
            }
            std::cout << "Received consecutive frame : { ";
            for (auto byte : data) {
                std::cout << static_cast<int>(byte) << " ";
            }
            std::cout << " } "<<std::endl;
            waitCount = 0;
            std::vector<uint8_t> consecutiveData = frameCanTP.GetDataFromConsecutiveFrame(data);
            buffer.insert(buffer.end(), consecutiveData.begin(), consecutiveData.end());
            framesReceivedInBlock++;
            if (blockSize != 0 && framesReceivedInBlock >= blockSize) {
                framesReceivedInBlock = 0;
                std::vector<uint8_t> fcData = frameCanTP.CreateFlowControlFrame(0x00, blockSize, separationTimeMs);
                FrameCAN fcFrame(frame_id, fcData);
                busManager.send(fcFrame);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(separationTimeMs));
        }
        if (buffer.size() > expectedSize) {
            buffer.resize(expectedSize); 
        }
    } else {
        std::cerr << "Unsupported or unexpected frame type: 0x" << std::hex << (int)frameType << std::dec << std::endl;
    }
    if (buffer.empty()) {
        std::cerr << "No data received after processing !!" << std::endl;
        return "";
    }
    std::cout << "Data received : ( " << buffer.size() << " bytes ) { " ;
    for (auto byte : buffer) {
        std::cout << static_cast<int>(byte) << " ";
    }
    std::cout << " } "<<std::endl;
    std::string receivedData = decoder(buffer);
    return receivedData;
}

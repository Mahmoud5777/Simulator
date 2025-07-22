#include "FrameCanTP.hpp"
#include <cstdint>
#include <vector>

std::vector<uint8_t> CreateConsecutiveFrame(const std::vector<uint8_t>& data, uint8_t sequenceNumber) {
    std::vector<uint8_t> frame;
    frame.push_back(0x20 | sequenceNumber); 
    frame.insert(frame.end(), data.begin(), data.end());
    return frame;
}

std::vector<uint8_t> CreateFirstFrame(const std::vector<uint8_t>& data, uint16_t dataSize) {
    std::vector<uint8_t> frame;
    frame.push_back(0x10 | ((dataSize >> 8) & 0x0F));
    frame.push_back(static_cast<uint8_t>(dataSize & 0xFF));
    frame.insert(frame.end(), data.begin(), data.end());
    return frame;
}

std::vector<uint8_t> CreateFlowControlFrame(uint8_t flowStatus, uint8_t blockSize, uint8_t separationTime) {
    std::vector<uint8_t> frame;
    frame.push_back(0x30 | flowStatus);
    frame.push_back(blockSize);
    frame.push_back(separationTime);
    return frame;
}

std::vector<uint8_t> CreateSingleFrame(const std::vector<uint8_t>& data,uint8_t dataSize) {
    std::vector<uint8_t> frame;
    frame.push_back(0x00 | (dataSize & 0x0F));
    frame.insert(frame.end(), data.begin(), data.end());
    return frame;
}

std::vector<uint8_t> GetDataFromFirstFrame(const std::vector<uint8_t>& data) {
    return std::vector<uint8_t>(data.begin() + 2, data.end());
}

std::vector<uint8_t> GetDataFromConsecutiveFrame(const std::vector<uint8_t>& data) {
    return std::vector<uint8_t>(data.begin() + 1, data.end());
}

std::vector<uint8_t> GetDataFromSingleFrame(const std::vector<uint8_t>& data) {
    return std::vector<uint8_t>(data.begin() + 1, data.end());
}

uint8_t GetBlockSizeFromFlowControlFrame(const std::vector<uint8_t>& data) {
    return data[1];
}

uint8_t GetFlagFromFlowControlFrame(const std::vector<uint8_t>& data) {
    return data[0] & 0x0F; 
}


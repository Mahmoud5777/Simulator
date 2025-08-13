#ifndef FRAMECANTP_HPP   
#define FRAMECANTP_HPP
#include <cstdint>
#include <vector>

class FrameCanTP {
    public:
        FrameCanTP() = default;
        ~FrameCanTP() = default;
        std::vector<uint8_t> CreateConsecutiveFrame(const std::vector<uint8_t>& data, uint8_t sequenceNumber);
        std::vector<uint8_t> CreateFirstFrame(const std::vector<uint8_t>& data ,uint16_t dataSize);
        std::vector<uint8_t> CreateFlowControlFrame(uint8_t flowStatus, uint8_t blockSize, uint8_t separationTime);
        std::vector<uint8_t> CreateSingleFrame(const std::vector<uint8_t>& data,uint8_t dataSize);
        std::vector<uint8_t> GetDataFromFirstFrame( const std::vector<uint8_t>& data);
        std::vector<uint8_t> GetDataFromConsecutiveFrame(const std::vector<uint8_t>& data);
        std::vector<uint8_t> GetDataFromSingleFrame(const std::vector<uint8_t>& data);
        uint8_t GetBlockSizeFromFlowControlFrame(const std::vector<uint8_t>& data); 
        uint8_t GetFlagFromFlowControlFrame(const std::vector<uint8_t>& data);
};

#endif
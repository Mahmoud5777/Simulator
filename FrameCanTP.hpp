#ifndef FRAMECANTP_HPP   
#define FRAMECANTP_HPP
#include <cstdint>
#include <vector>

class FrameCanTP {
    public:
        FrameCanTP() = default;
        ~FrameCanTP() = default;
        void CreateFirstFrame(const std::vector<uint8_t>& data);
        void CreateConsecutiveFrame(const std::vector<uint8_t>& data, uint8_t sequenceNumber);
        void CreateFlowControlFrame(uint8_t flowStatus, uint8_t blockSize, uint8_t separationTime);
        void CreateSingleFrame(const std::vector<uint8_t>& data);
        std::vector<uint8_t> GetDataFromFirstFrame();
        std::vector<uint8_t> GetDataFromConsecutiveFrame(uint8_t sequenceNumber);
        std::vector<uint8_t> GetDataFromSingleFrame();
        uint8_t GetFlagFromFlowControlFrame();
        uint8_t GetBlockSizeFromFlowControlFrame(); 
};

#endif
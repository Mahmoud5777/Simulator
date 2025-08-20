#ifndef FRAMECAN_H
#define FRAMECAN_H
#include <vector>
#include <cstdint>
#include "ID.hpp"

class FrameCAN {
private:
    uint16_t frameID;
    std::vector<uint8_t> data;

public:
    FrameCAN(const uint16_t id, const std::vector<uint8_t>& data) 
        : frameID(id), data(data) {}

    // Getters
    ID getFrameID() const { return frameID; }
    std::vector<uint8_t> getData() const { return data; }

    // Setter pour data
    void setData(const std::vector<uint8_t>& newData) { data = newData; }
};
#endif

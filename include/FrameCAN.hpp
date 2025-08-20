#ifndef FRAMECAN_H
#define FRAMECAN_H
#include <vector>
#include <cstdint>
#include "ID.hpp"

class FrameCAN {
private:
    ID frameID;
    std::vector<uint8_t> data;

public:
    // Toujours passer un ID valide
    FrameCAN(const ID& id, const std::vector<uint8_t>& data)
        : frameID(id), data(data) {}

    // Getters
    const ID& getFrameID() const { return frameID; }
    const std::vector<uint8_t>& getData() const { return data; }

    // Setter pour data
    void setData(const std::vector<uint8_t>& newData) { data = newData; }
};

#endif

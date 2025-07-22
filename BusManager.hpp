#ifndef BusManager_HPP
#define BusManager_HPP
#include <vector>
#include <cstdint>
#include "FrameCAN.hpp"

class BusManager {
public:
    BusManager() = default;
    ~BusManager() = default;
    void send(const FrameCAN &frame) {}
    FrameCAN receive() {}
};

#endif
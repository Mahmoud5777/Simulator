#ifndef BUSMANAGER_HPP
#define BUSMANAGER_HPP

#include <vector>
#include <cstdint>
#include "FrameCAN.hpp"


class BusManager {
public:
    BusManager();
    ~BusManager();
    bool createVCAN();
    bool init();
    void send(const FrameCAN &frame);
    FrameCAN receive();
    void closeSocket();

private:
#ifdef __linux__
    int socket_fd = -1; // Descripteur du socket CAN
#endif
};

#endif // BUSMANAGER_HPP

#ifndef BUSMANAGER_HPP
#define BUSMANAGER_HPP

#include <vector>
#include <cstdint>
#include "FrameCAN.hpp"


class BusManager {
public:
    BusManager();
    virtual ~BusManager();
    bool createVCAN();
    bool init();
    virtual void send(const FrameCAN &frame);
    virtual FrameCAN receive();
    void closeSocket();

private:
#ifdef __linux__
    int socket_fd = -1; // Descripteur du socket CAN
#endif
};

#endif // BUSMANAGER_HPP

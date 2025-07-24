#include "BusManager.hpp"
#include "FrameCAN.hpp"
#include <iostream>

#ifdef __linux__
#include <cstdlib> 
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#endif

BusManager::~BusManager() {
    closeSocket();
}

bool BusManager::createVCAN() {
    #ifdef __linux__
        int ret = 0;
        ret = system("modprobe vcan");
        if (ret != 0) {
            std::cerr << "Erreur modprobe vcan\n";
            return false;
        }
        system("ip link delete vcan0 2>/dev/null");
        ret = system("ip link add dev vcan0 type vcan");
        if (ret != 0) {
            std::cerr << "Erreur création interface vcan0\n";
            return false;
        }
        ret = system("ip link set up vcan0");
        if (ret != 0) {
            std::cerr << "Erreur activation interface vcan0\n";
            return false;
        }
        std::cout << "Interface vcan0 créée et activée\n";
        return true;
    #else
        std::cerr << "createVCAN() only supported on Linux." << std::endl;
        return false;
    #endif
}


bool BusManager::init() {
#ifdef __linux__
    struct ifreq ifr{};
    struct sockaddr_can addr{};

    socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_fd < 0) {
        perror("Erreur création socket CAN");
        return false;
    }

    std::strncpy(ifr.ifr_name, "vcan0", IFNAMSIZ - 1);
    if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
        perror("Erreur ioctl");
        close(socket_fd);
        socket_fd = -1;
        return false;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Erreur bind");
        close(socket_fd);
        socket_fd = -1;
        return false;
    }

    std::cout << "Socket CAN initialisée sur vcan0\n";
    return true;
#else
    std::cerr << "Plateforme non supportée\n";
    return false;
#endif
}

void BusManager::send(const FrameCAN& trame) {
#ifdef __linux__
    if (socket_fd < 0) {
        std::cerr << "Socket CAN non initialisée\n";
        return;
    }

    struct can_frame frame{};
    frame.can_id = trame.getFrameID();    // suppose que FrameCAN a getId()
    frame.can_dlc = 8 ;  // suppose que FrameCAN a getDLC()
    std::memcpy(frame.data, trame.getData().data(), frame.can_dlc);

    if (write(socket_fd, &frame, sizeof(frame)) != sizeof(frame)) {
        perror("Erreur d'envoi CAN");
    } else {
        std::cout << "Trame CAN envoyée (ID: 0x" << std::hex << frame.can_id << std::dec << ", " << (int)frame.can_dlc << " octets)\n";
    }
#endif
}

FrameCAN BusManager::receive() {
#ifdef __linux__
    if (socket_fd < 0) {
        std::cerr << "Socket CAN non initialisée pour réception\n";
        return FrameCAN();  // FrameCAN vide
    }

    struct can_frame canFrame{};
    ssize_t nbytes = read(socket_fd, &canFrame, sizeof(canFrame));

    if (nbytes < 0) {
        perror("Erreur lecture CAN");
        return FrameCAN();
    }

    if (nbytes < sizeof(canFrame)) {
        std::cerr << "Trame CAN incomplète reçue\n";
        return FrameCAN();
    }

    return FrameCAN(canFrame.can_id,
                    std::vector<uint8_t>(canFrame.data, canFrame.data + canFrame.can_dlc));
#else
    std::cerr << "Réception CAN non supportée sur cette plateforme\n";
    return FrameCAN();
#endif
}

void BusManager::closeSocket() {
#ifdef __linux__
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
        std::cout << "Socket CAN fermée\n";
    }
#endif
}

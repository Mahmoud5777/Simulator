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

BusManager::BusManager() {
#ifdef __linux__   
    socket_fd = -1;
#endif
}

BusManager::~BusManager() {
    closeSocket();
}

bool BusManager::createVCAN() {
#ifdef __linux__
    // Mode silencieux pour la simulation
    system("modprobe vcan >/dev/null 2>&1");
    system("modprobe can_raw >/dev/null 2>&1");
    system("ip link delete vcan0 >/dev/null 2>&1");
    system("ip link add dev vcan0 type vcan >/dev/null 2>&1");
    system("ip link set up vcan0 >/dev/null 2>&1");
    
    std::cout << "[SIM] Interface vcan0 créée (mode simulation)" << std::endl;
    return true;
#else
    std::cerr << "CAN simulé seulement sous Linux" << std::endl;
    return false;
#endif
}

bool BusManager::init() {
#ifdef __linux__
    socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_fd < 0) {
        std::cerr << "[SIM] Erreur socket (simulation)" << std::endl;
        return false;
    }

    // Configuration simplifiée
    struct ifreq ifr = {};
    strncpy(ifr.ifr_name, "vcan0", IFNAMSIZ);
    ioctl(socket_fd, SIOCGIFINDEX, &ifr);

    struct sockaddr_can addr = {};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr));

    std::cout << "[SIM] Socket CAN-TP initialisée (simulation)" << std::endl;
    return true;
#else
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
    }
#endif
}
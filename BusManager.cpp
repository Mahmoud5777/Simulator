#include "BusManager.hpp"
#include "FrameCAN.hpp"
#include <iostream>

#ifdef __linux__
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#endif

// Constructeur
BusManager::BusManager() : socket_fd(-1) {}

// Destructeur
BusManager::~BusManager() {
    closeSocket();
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
    
    // Valeurs statiques utilisées ici :
    frame.can_id = 0x123;              // ID CAN fixe
    frame.can_dlc = 8;                 // DLC fixe (8 octets)
    std::vector<uint8_t> fakeData = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

    std::memcpy(frame.data, fakeData.data(), frame.can_dlc);

    if (write(socket_fd, &frame, sizeof(frame)) != sizeof(frame)) {
        perror("Erreur d'envoi CAN");
    } else {
        std::cout << "Trame CAN envoyée (ID: 0x123, 8 octets)\n";
    }
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

FrameCAN BusManager::receive() {
#ifdef __linux__
    if (socket_fd < 0) {
        std::cerr << "Socket CAN non initialisée pour réception\n";
        return FrameCAN();
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

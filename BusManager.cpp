#include "BusManager.hpp"
#include "FrameCAN.hpp"
#ifdef __linux__
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#endif

bool BusManager::init() {
#ifdef __linux__
    struct ifreq ifr{};
    struct sockaddr_can addr{};
    // 1. Création du socket CAN
    socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_fd < 0) {
        perror("Erreur création socket CAN");
        return false;
    }
    // 2. Récupération de l'index de l'interface "vcan0"
    std::strncpy(ifr.ifr_name, "vcan0", IFNAMSIZ - 1);
    if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
        perror("Erreur ioctl");
        close(socket_fd);
        socket_fd = -1;
        return false;
    }
    // 3. Préparation de l'adresse CAN
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    // 4. Liaison du socket à l'interface CAN
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
    frame.can_id = trame.getId();
    frame.can_dlc = trame.getDLC();
    std::memcpy(frame.data, trame.getData().data(), frame.can_dlc);

    if (write(socket_fd, &frame, sizeof(frame)) != sizeof(frame)) {
        perror("Erreur d'envoi CAN");
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
        return FrameCAN(); // Retourne une FrameCAN vide ou par défaut
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
    // Construire un FrameCAN à partir des données reçues
    return FrameCAN(canFrame.can_id,
                    std::vector<uint8_t>(canFrame.data, canFrame.data + canFrame.can_dlc));
#else
    std::cerr << "Réception CAN non supportée sur cette plateforme\n";
    return FrameCAN();
#endif
}



#include "../include/BusManager.hpp"
#include "../include/FrameCAN.hpp"
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
#include <cstdint>
#endif

BusManager::BusManager() {
#ifdef __linux__   
    socket_fd = -1; 
#endif
}

BusManager::~BusManager() {
    closeSocket();
}

/*bool BusManager::createVCAN() {
#ifdef __linux__
    // 1. Chargement du module noyau
    if (system("modprobe -q vcan && modprobe -q can_raw") != 0) {
        std::cerr << "Erreur: Impossible de charger les modules noyau vcan/can_raw" << std::endl;
        return false;
    }

    // 2. Suppression de l'interface existante (silencieuse)
    system("ip link delete vcan0 2>/dev/null");

    // 3. Création de l'interface
    if (system("ip link add dev vcan0 type vcan") != 0) {
        std::cerr << "Erreur: Échec de création de vcan0" << std::endl;
        return false;
    }

    // 4. Activation de l'interface
    if (system("ip link set up vcan0") != 0) {
        std::cerr << "Erreur: Échec d'activation de vcan0" << std::endl;
        // Nettoyage en cas d'échec
        system("ip link delete vcan0 2>/dev/null");
        return false;
    }

    // Vérification finale
    if (system("ip link show vcan0 >/dev/null 2>&1") != 0) {
        std::cerr << "Erreur: Vérification de vcan0 échouée" << std::endl;
        return false;
    }

    std::cout << "Success: Interface vcan0 créée et activée" << std::endl;
    return true;
#else
    std::cerr << "Erreur: createVCAN() n'est supporté que sous Linux" << std::endl;
    return false;
#endif
}
*/

bool BusManager::init() {
#ifdef __linux__
    struct ifreq ifr{}; //Interface Request
    struct sockaddr_can addr{}; // socket CAN Address
    socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW); // Création du socket CAN
    if (socket_fd < 0) {
        perror("CAN socket creation error");
        return false;
    }
    std::strncpy(ifr.ifr_name, "vcan0", IFNAMSIZ - 1);// Nom de l'interface CAN
    if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {// Récupération de l'index de l'interface
        perror("Error ioctl");
        close(socket_fd);
        socket_fd = -1;
        return false;
    }
    addr.can_family = AF_CAN; // Type de famille d'adresse
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {// Liaison du socket à l'interface
        perror("Error bind");
        close(socket_fd);
        socket_fd = -1;
        return false;
    }
    return true;
#else
    std::cerr << "Platform not supported\n";
    return false;
#endif
}

void BusManager::send(const FrameCAN& trame) {
#ifdef __linux__
    if (socket_fd < 0) {
        std::cerr << "CAN socket not initialized !\n";
        return;
    }
    struct can_frame frame{};
    uint32_t raw_id = trame.getFrameID().getID(); // Récupération de l'ID CAN
    frame.can_id = trame.getFrameID().isExtended()? (raw_id | CAN_EFF_FLAG) : raw_id; 
    frame.can_dlc = trame.getData().size(); 
    std::memcpy(frame.data, trame.getData().data(), frame.can_dlc);
    if (write(socket_fd, &frame, sizeof(frame)) != sizeof(frame)) {
        perror("CAN sending error !");
    }else{
        std::cout << "CAN frame sent (ID: 0x" << std::hex << frame.can_id << std::dec << ", " << (int)frame.can_dlc << " octets)\n";
    }
#endif
}

FrameCAN BusManager::receive() {
#ifdef __linux__
    if (socket_fd < 0) {
        std::cerr << "CAN socket not initialized for reception !\n";
        return FrameCAN(); 
    }
    struct can_frame canFrame{};
    ssize_t nbytes = read(socket_fd, &canFrame, sizeof(canFrame));
    if (nbytes < 0) {
        perror("CAN reading error");
        return FrameCAN();
    }
    if (nbytes < sizeof(canFrame)) {
        std::cerr << "Incomplete CAN frame received\n";
        return FrameCAN();
    }
    return FrameCAN(ID(canFrame.can_id & CAN_EFF_FLAG ? (canFrame.can_id & CAN_EFF_MASK) : canFrame.can_id),std::vector<uint8_t>(canFrame.data, canFrame.data + canFrame.can_dlc));
#else
    std::cerr << "CAN reception not supported on this platform\n";
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
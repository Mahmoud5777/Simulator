// EcuSender.cpp
#include "EcuSender.hpp"
#include <iostream>
#include <string>
#include "CanManager.hpp"

EcuSender::EcuSender(BusManager& busManager)
    : bus(busManager) {}

void EcuSender::run() {
    std::cout << "EcuSender is running." << std::endl;
    std::string data;
    std::cout << "Enter data to send: ";
    std::getline(std::cin >> std::ws, data);  // permet de lire une ligne complète avec espaces

    CanManager canManager(bus);  // passe la référence BusManager à CanManager
    canManager.send(data);
}

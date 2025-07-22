// EcuReceiver.cpp
#include "EcuReceiver.hpp"
#include <iostream>
#include "CanManager.hpp"

EcuReceiver::EcuReceiver(BusManager& busManager)
    : bus(busManager) {}

void EcuReceiver::run() {
    std::cout << "EcuReceiver is running." << std::endl;
    CanManager canManager(bus);  // Passe la référence ici
    std::string data = canManager.receive();
    std::cout << "Received data: " << data << std::endl;
}

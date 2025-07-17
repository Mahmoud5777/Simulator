# include "EcuReceiver.hpp"
#include <iostream>
#include <string>
#include "CanManager.hpp"

EcuReceiver::EcuReceiver(const std::string& id) : Ecu(id) {}

void EcuReceiver::run() {
    std::cout << "EcuReceiver is running." << std::endl;
    std::string data;
    CanManager CanManger;
    CanManger.receive(data);
    std::cout << "Received data: " << data << std::endl;
}
# include "EcuReceiver.hpp"
#include <iostream>
#include <string>
#include "CanManager.hpp"


void EcuReceiver::run() {
    std::cout << "EcuReceiver is running." << std::endl;
    std::string data;
    CanManager CanManger;
    data = CanManger.receive();
    std::cout << "Received data: " << data << std::endl;
}
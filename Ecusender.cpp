#include "Ecusender.hpp"
#include <iostream>
#include <string>
#include "CanManager.hpp" 

EcuSender::EcuSender(const std::string& id) : Ecu(id) {}

void EcuSender::run() {
    std::cout << "EcuSender is running." << std::endl;
    std::string data;
    std::cout << "Enter data to send: ";
    std::cin >> data;
    CanManager CanManger;
    CanManger.send(data);
}
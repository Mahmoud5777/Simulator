#include "Ecusender.hpp"
#include <iostream>
#include <string>
#include "CanManager.hpp" 



void EcuSender::run() {
    std::cout << "EcuSender is running." << std::endl;
    std::string data;
    std::cout << "Enter data to send: ";
    std::cin >> data;
    CanManager CanManger;
    CanManger.send(data);
}
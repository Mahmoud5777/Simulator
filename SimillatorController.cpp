#include "SimillatorController.hpp"
#include <iostream>
#include <string>
#include "EcuSender.hpp"
#include "EcuReceiver.hpp"

void SimillatorController::run() {
    std::cout << "Simulator is running." << std::endl;
    std::cout << "Would you like to send or receive data? (send/receive): ";
    std::string choice;
    std::cin >> choice;

    if (choice == "send") {
        std::cout << "Give your ID" << std::endl;
        //std::string &id;
        //std::cin >> id;
        EcuSender sender("gg");
        sender.run();

    } else if (choice == "receive") {
        std::cout << "Give your ID" << std::endl;
        //std::string &id;
        //std::cin >> id;
        EcuReceiver receiver("ff");
        receiver.run();

    } else {
        std::cout << "Invalid choice. Please enter 'send' or 'receive'." << std::endl;
    }
}
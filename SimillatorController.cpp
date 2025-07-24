#include "SimillatorController.hpp"
#include <iostream>
#include <string>
#include "EcuSender.hpp"
#include "EcuReceiver.hpp"
#include "BusManager.hpp"

SimillatorController::SimillatorController(BusManager& busManager)
    : busManager(busManager) {}

void SimillatorController::run() {
    std::cout << "Simulator is running." << std::endl;
    std::cout << "Would you like to send (1) or receive (2) data ? (1/2): ";
    int choice;
    while (choice != 0) {
        std::cin >> choice;
        std::cin.ignore();
        if (choice == 1) {
            std::cout << "Give your ID" << std::endl;
            std::string id;
            std::cin >> id;
            std::cin.ignore();
            EcuSender sender(busManager);   // passe la référence BusManager à EcuSender
            sender.run();
        }else if (choice == 2) {
            std::cout << "Give your ID" << std::endl;
            std::string id;
            std::cin >> id;
            std::cin.ignore();
            EcuReceiver receiver(busManager);  // passe la référence BusManager à EcuReceiver
            receiver.run();
        } else {
            std::cout << "Invalid choice. Please enter '1' or '2' or '0' to exit the simulator " << std::endl;
        }
    }
}


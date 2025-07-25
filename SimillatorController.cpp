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
    int choice = -1;
    int send = 0;
    int receive = 0;
    while (choice != 0) {
        if (send > 0) {
            std::cout << "Choose 1 to continue sending data or 0 to exit : ";
        } else if (receive > 0) {
            std::cout << "Choose 2 to continue receiving data or 0 to exit : ";
        }
        std::cin >> choice;
        std::cin.ignore();
        if (choice == 1) {
            send++;
            std::cout << "Give your ID : " ;
            std::string id;
            std::cin >> id;
            std::cin.ignore();
            EcuSender sender(busManager);   // passe la référence BusManager à EcuSender
            sender.run();
        }else if (choice == 2) {
            receive++;
            std::cout << "Give your ID : " ;
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


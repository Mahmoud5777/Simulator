#include "SimillatorController.hpp"
#include <iostream>
#include <string>
#include "EcuSender.hpp"
#include "EcuReceiver.hpp"

SimillatorController::SimillatorController(BusManager& busManager)
    : bus(busManager) {
}

void SimillatorController::run() {
    std::cout << "Simulator is running." << std::endl;
    std::cout << "Would you like to send or receive data? (send/receive): ";
    std::string choice;
    std::cin >> choice;

    if (choice == "send") {
        std::cout << "Give your ID" << std::endl;
        // tu peux récupérer l'ID ici si besoin
        EcuSender sender(bus);   // passe la référence bus ici
        sender.run();
    } else if (choice == "receive") {
        std::cout << "Give your ID" << std::endl;
        // pareil pour ID si besoin
        EcuReceiver receiver(bus);  // passe la référence bus ici
        receiver.run();
    } else {
        std::cout << "Invalid choice. Please enter 'send' or 'receive'." << std::endl;
    }
}

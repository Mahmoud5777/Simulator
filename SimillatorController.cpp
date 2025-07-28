#include "SimillatorController.hpp"
#include <iostream>
#include "EcuSender.hpp"
#include "EcuReceiver.hpp"
#include "BusManager.hpp"

SimillatorController::SimillatorController(BusManager& busManager)
    : busManager(busManager) {}

void SimillatorController::run() {
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "         🚘 CAN TP Virtual Simulator           \n";
    std::cout << "═══════════════════════════════════════════════\n";

    int choice = -1;
    int send = 0;
    int receive = 0;

    while (true) {
        std::cout << "\n Main menu :\n";
        std::cout << " ┌────────────────────────────────────────────┐\n";
        std::cout << " │ [1] Send a CAN frame                       │\n";
        std::cout << " │ [2] Receive a CAN frame                    │\n";
        std::cout << " │ [0] exit                                   │\n";
        std::cout << " └────────────────────────────────────────────┘\n";
        std::cout << " 👉 Your choice : ";

        if (!(std::cin >> choice)) {
            std::cin.clear(); // Nettoyer l'état d'erreur
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignorer la mauvaise saisie
            std::cout << " Invalid entry. Please enter a number (0, 1 or 2).\n";
            continue;
        }

        std::cin.ignore(); // Supprimer le retour à la ligne

        if (choice == 1) {
            send++;
            std::cout << "\n──────────────────────────────────────────────\n";
            EcuSender sender(busManager); // passe la référence
            sender.run();
            int subChoice;
            std::cout << "\n 🔁 Continue sending? (1 = Yes, other = Return to menu) : ";
            std::cin >> subChoice;
            if (subChoice != 1) continue;
        }

        else if (choice == 2) {
            receive++;
            std::cout << "\n──────────────────────────────────────────────\n";
            EcuReceiver receiver(busManager);
            receiver.run();
            int subChoice;
            std::cout << "\n 🔁 Continue receiving? (2 = Yes, other = Return to menu) : ";
            std::cin >> subChoice;
            if (subChoice != 2) continue;
        }

        else if (choice == 0) {
            std::cout << "═════════════════════════════════════════════════════════════\n";
            std::cout << "                       End of simulation.                     \n";
            std::cout << "═════════════════════════════════════════════════════════════\n";
            break;
        }

        else {
            std::cout << " Invalid choice. Please enter 0, 1, or 2.\n";
        }
    }
}



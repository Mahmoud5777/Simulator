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
        std::cout << "\n Menu principal :\n";
        std::cout << " ┌────────────────────────────────────────────┐\n";
        std::cout << " │ [1] Envoyer une trame CAN                  │\n";
        std::cout << " │ [2] Recevoir une trame CAN                 │\n";
        std::cout << " │ [0] Quitter                                │\n";
        std::cout << " └────────────────────────────────────────────┘\n";
        std::cout << " 👉 Votre choix : ";

        if (!(std::cin >> choice)) {
            std::cin.clear(); // Nettoyer l'état d'erreur
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignorer la mauvaise saisie
            std::cout << " Entrée invalide. Veuillez entrer un chiffre (0, 1 ou 2).\n";
            continue;
        }

        std::cin.ignore(); // Supprimer le retour à la ligne

        if (choice == 1) {
            send++;
            std::cout << "\n──────────────────────────────────────────────\n";
            EcuSender sender(busManager); // passe la référence
            sender.run();
            int subChoice;
            std::cout << "\n 🔁 Continuer à envoyer ? (1 = Oui, autre = Retour au menu) : ";
            std::cin >> subChoice;
            if (subChoice != 1) continue;
        }

        else if (choice == 2) {
            receive++;
            std::cout << "\n──────────────────────────────────────────────\n";
            EcuReceiver receiver(busManager);
            receiver.run();
            int subChoice;
            std::cout << "\n 🔁 Continuer à recevoir ? (2 = Oui, autre = Retour au menu) : ";
            std::cin >> subChoice;
            if (subChoice != 2) continue;
        }

        else if (choice == 0) {
            std::cout << "\n Fin de simulation. Merci d'avoir utilisé le simulateur.\n";
            break;
        }

        else {
            std::cout << " Choix invalide. Veuillez entrer 0, 1 ou 2.\n";
        }
    }
}



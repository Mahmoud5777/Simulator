#include <iostream>
#include "../include/SimillatorController.hpp"
#include "../include/BusManager.hpp"

int main() {
    BusManager busManager;
    /*if (!busManager.createVCAN()) {
        std::cerr << "Failed to create vCAN interface." << std::endl;
        return 1;
    }*/
    if (!busManager.init()) {
        std::cerr << "Failed to initialize BusManager." << std::endl;
        return 1;
    }
    SimillatorController simulator(busManager); 
    simulator.run();
    busManager.closeSocket();
    return 0;
}

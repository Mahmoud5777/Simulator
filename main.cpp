# include <iostream>
# include "SimillatorController.hpp"
# include "BusManager.hpp"
int main() {
    // Initialisation du BusManager
    BusManager busManager;
    if (!busManager.init()) {
        std::cerr << "Failed to initialize BusManager." << std::endl;
        return 1;
    }
    SimillatorController simulator;
    simulator.run();
    
    return 0;
}
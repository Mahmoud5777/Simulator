#include <iostream>
#include "SimillatorController.hpp"
#include "BusManager.hpp"

int main() {
    BusManager busManager;
    if (!busManager.init()) {
        std::cerr << "Failed to initialize BusManager." << std::endl;
        return 1;
    }
    SimillatorController simulator(busManager);  // Passe la référence
    simulator.run();
    
    return 0;
}

#ifndef SIMILLATORCONTROLLER_HPP
#define SIMILLATORCONTROLLER_HPP

#include "BusManager.hpp"

class SimillatorController {
public:
    SimillatorController(BusManager& busManager);
    ~SimillatorController() = default;
    void run();
private:
    BusManager& bus;  // référence au busManager
};

#endif

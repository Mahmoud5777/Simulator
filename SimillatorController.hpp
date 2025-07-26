#ifndef SIMILLATORCONTROLLER_HPP
    #define SIMILLATORCONTROLLER_HPP
    #include "BusManager.hpp"
    #include "ID.hpp"

    class SimillatorController {
    public:
        explicit SimillatorController(BusManager& busManager);
        ~SimillatorController() = default;
        ID getEcuID() const { return ecuID; } 
        void run();
    private:
        BusManager busManager;  // Référence au BusManager partagé
    }

#endif

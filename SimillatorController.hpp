#ifndef SIMILLATORCONTROLLER_HPP
    #define SIMILLATORCONTROLLER_HPP
    #include "BusManager.hpp"

    class SimillatorController {
    public:
        explicit SimillatorController(BusManager& busManager);
        ~SimillatorController() = default;
        void run();
    private:
        BusManager busManager;     // Référence au BusManager partagé
    };

#endif

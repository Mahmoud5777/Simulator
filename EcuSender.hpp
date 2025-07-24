#ifndef ECUSENDER_HPP
    #define ECUSENDER_HPP
    #include "BusManager.hpp"

    class EcuSender {
    public:
        explicit EcuSender(BusManager& busManager); 
        void run();

    private:
        BusManager& bus;  // référence au BusManager partagé
    };

#endif
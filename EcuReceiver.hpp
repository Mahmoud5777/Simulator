// EcuReceiver.hpp
#ifndef ECURECEIVER_HPP
    #define ECURECEIVER_HPP
    #include <string>
    #include "BusManager.hpp"

    class EcuReceiver {
    public:
        explicit EcuReceiver(BusManager& bus);
        void run();

    private:
        BusManager& bus;
    };

#endif

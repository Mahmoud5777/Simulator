#ifndef ECURECIEVER_HPP
#define ECURECIEVER_HPP
#include "Ecu.hpp"

class EcuReceiver : public Ecu {
    public:
        EcuReceiver(const std::string& id) : Ecu(id) {}
        void run () override;
};

#endif
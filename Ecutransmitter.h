#ifndef ECUTRANSMITTER_H
#define ECUTRANSMITTER_H
#include "Ecu.h"
#include <string>
#include <iostream>
#include "FrameCAN.h"

class EcuTransmitter : public Ecu {
    private:
        FrameCAN frame;
    public:
       // FrameCAN::FrameCAN reciveData() override;
        bool sendData(const std::string& data) override;
};

#endif
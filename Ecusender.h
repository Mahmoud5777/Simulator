#ifndef ECUSENDER_H
#define ECUSENDER_H
#include "Ecu.h"
#include <string>
#include <iostream>

class EcuSender : public Ecu {
    public:
        std::string reciveData() override;
        bool sendData(const std::string& data) override;
};

#endif
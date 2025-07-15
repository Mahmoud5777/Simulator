# include "Ecu.h"
#include <iostream>

Ecu ::Ecu(const std::string& name, const std::string& id) 
    : ECUName(name), ECUID(id) {
        Ecu::ECUName = name;
        Ecu::ECUID = id;
}

std::string Ecu::getECUID() const {
    return ECUID;
}


# include "Ecu.hpp"
#include <iostream>

Ecu ::Ecu( const std::string& id) 
    : ECUID(id) {
        Ecu::ECUID = id;
}

std::string Ecu::getECUID() const {
    return ECUID;
}


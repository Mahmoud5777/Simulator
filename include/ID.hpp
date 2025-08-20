#ifndef ID_HPP
#define ID_HPP

#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

class ID {
private:
    uint16_t canIdRx_;
    uint16_t canIdTx_;

public:
    // Chargement depuis JSON
    static ID loadFromJson(const std::string& jsonFile) {
        std::ifstream file(jsonFile);
        if (!file.is_open()) throw std::runtime_error("Impossible d'ouvrir " + jsonFile);

        json config;
        file >> config;
        if (!config.contains("CanIdRx") || !config.contains("CanIdTx"))
            throw std::runtime_error("Clés CanIdRx ou CanIdTx manquantes");

        uint16_t rx = std::stoul(config["CanIdRx"].get<std::string>(), nullptr, 16);
        uint16_t tx = std::stoul(config["CanIdTx"].get<std::string>(), nullptr, 16);
        if (rx > 0x7FF || tx > 0x7FF)
            throw std::invalid_argument("IDs doivent être standards");

        return ID(rx, tx);
    }

    buildSmartID() {
        // Crée un ID standard pour les tests
        return ID(0x700, 0x700);
    }

    // Constructeur
    ID(uint16_t rx, uint16_t tx) : canIdRx_(rx), canIdTx_(tx) {}

    uint16_t getRx() const { return canIdRx_; }
    uint16_t getTx() const { return canIdTx_; }
};

#endif

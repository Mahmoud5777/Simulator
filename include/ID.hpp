#ifndef ID_HPP
#define ID_HPP

#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include "json.hpp"  // nlohmann/json

using json = nlohmann::json;

class ID {
private:
    uint16_t canIdRx_;
    uint16_t canIdTx_;

    // Constructeur privé
    ID(uint16_t rx, uint16_t tx) : canIdRx_(rx), canIdTx_(tx) {}

public:
    // Interdire la création d'objet vide ou manuel
    ID() = delete;

    // Charger les deux IDs depuis le fichier JSON
    static ID loadFromJson(const std::string& jsonFile) {
        std::ifstream file(jsonFile);
        if (!file.is_open()) {
            throw std::runtime_error("Impossible d'ouvrir " + jsonFile);
        }

        json config;
        file >> config;

        // Vérifier les clés
        if (!config.contains("CanIdRx") || !config.contains("CanIdTx")) {
            throw std::runtime_error("Clés CanIdRx ou CanIdTx manquantes dans le JSON");
        }

        // Conversion des IDs hex en int
        uint16_t rx = std::stoul(config["CanIdRx"].get<std::string>(), nullptr, 16);
        uint16_t tx = std::stoul(config["CanIdTx"].get<std::string>(), nullptr, 16);

        // Vérifier que ce sont des IDs standards (≤ 0x7FF)
        if (rx > 0x7FF || tx > 0x7FF) {
            throw std::invalid_argument("Les IDs doivent être standards (<= 0x7FF)");
        }

        return ID(rx, tx);
    }

    // Getters
    uint16_t getRx() const { return canIdRx_; }
    uint16_t getTx() const { return canIdTx_; }

    ~ID() = default;
};

#endif

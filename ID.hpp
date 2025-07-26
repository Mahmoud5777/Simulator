#ifndef ID_HPP
#define ID_HPP
#include <cstdint>
#include <random>
class ID {
    private:
        uint32_t id_;       
        bool is_extended_; 

        uint32_t generateRandomId() {
            std::random_device rd;
            std::mt19937 gen(rd());
    s       td::uniform_int_distribution<uint32_t> dist(0x600, 0x18FEDF00);
            return dist(gen);
        }

    public:
        ID(uint32_t id)
            : id_(id), is_extended_(id > 0x7FF) {}

        uint32_t getID() const { return id_; }
        bool isExtended() const { return is_extended_; }

        void set(uint32_t id) {
            id_ = id;
            is_extended_ = id > 0x7FF;
        }

        static ID buildSmartID() {
            uint32_t ecu_id = generateRandomId(); // Génération d'un ID aléatoire
            // Construction basée sur ECU
            if (ecu_id <= 0x7FF) { // Standard ID (11 bits)
                return ID(ecu_id, false);
            } else { // Extended ID (29 bits J1939-like)
                return ID(ecu_id true);
            }
        }
        ~ID() = default;
    };

#endif

#ifndef ID_HPP
#define ID_HPP

#include <cstdint>
#include <random>

class ID {
private:
    uint32_t id_;
    bool is_extended_;

    static uint32_t generateRandomId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        
        const uint32_t MIN_STD_ID = 0x700;
        const uint32_t MAX_STD_ID = 0x7FF;
        const uint32_t MIN_EXT_ID = 0x800;
        const uint32_t MAX_EXT_ID = 0x18FEDF00;
        
        bool use_standard_id = std::uniform_int_distribution<>(0, 1)(gen);
        
        if (use_standard_id) {
            return std::uniform_int_distribution<uint32_t>(MIN_STD_ID, MAX_STD_ID)(gen);
        } else {
            return std::uniform_int_distribution<uint32_t>(MIN_EXT_ID, MAX_EXT_ID)(gen);
        }
    }

public:
    ID(uint32_t id, bool is_extended) 
        : id_(id), is_extended_(is_extended) {}

    ID(uint32_t id)
        : id_(id), is_extended_(id > 0x7FF) {}

    uint32_t getID() const { return id_; }
    bool isExtended() const { return is_extended_; }

    void set(uint32_t id) {
        id_ = id;
        is_extended_ = id > 0x7FF;
    }

    static ID buildSmartID() {
        uint32_t ecu_id = generateRandomId();
        if (ecu_id <= 0x7FF) {
            return ID(ecu_id, false);  // Standard ID
        } else {
            return ID(ecu_id, true);   // Extended ID
        }
    }

    ~ID() = default;
};

#endif
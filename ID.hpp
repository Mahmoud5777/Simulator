#ifndef ID_HPP
#define ID_HPP
#include <cstdint>

class ID{
    public:
    ID(uint16_t id) : id_(id) {}
    uint16_t getID() const { return id_; }
    ~ID();
};

#endif 
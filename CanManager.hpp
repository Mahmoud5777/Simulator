#ifndef CANMANGER_HPP
    #define CANMANGER_HPP
    #include <string>
    #include <vector>
    #include "FrameCAN.hpp"
    class CanManager {
    public:
        explicit CanManager(BusManager &bus) : bus(busManager) {};
        ~CanManager() = default;
        void send(const std::string & frame);
        std::string receive();
    private:
        BusManager& busManager; 
        std::vector<uint8_t> encoder(const std::string & data);
        std::string decoder(const std::vector<uint8_t> & data);
    };

#endif
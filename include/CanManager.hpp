#ifndef CANMANGER_HPP
    #define CANMANGER_HPP
    #include <string>
    #include <vector>
    #include "FrameCAN.hpp"
    #include "BusManager.hpp"
    class CanManager {
    public:
        explicit CanManager(BusManager &bus) :busManager(bus) {};
        ~CanManager() = default;
        void send(const std::string & frame);
        std::string receive();
    private:
        BusManager& busManager; 
        std::vector<uint8_t> encoder(const std::string & data);
        std::string decoder(const std::vector<uint8_t> & data);
        uint8_t loadBlockSize(const std::string &jsonFile);
        uint8_t loadSeparationTime(const std::string &jsonFile);
    };

#endif
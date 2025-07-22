#ifndef CANMANGER_HPP
    #define CANMANGER_HPP
    #include <string>
    #include <vector>
    #include "FrameCAN.hpp"
    class CanManager {
    public:
        CanManager() = default;
        ~CanManager() = default;
        void send(const std::string & frame);
        std::string receive();
    private:
        std::vector<uint8_t> encoder(const std::string & data);
        std::string decoder(const std::vector<uint8_t> & data);
    };

#endif;
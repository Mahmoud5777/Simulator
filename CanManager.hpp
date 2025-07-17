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
        void receive(const std::string & frame);
    private:
        std::vector<uint8_t> encoder(const std::string & data);
        std::string decoder(const std::vector<uint8_t> & data);
        FrameCAN buildFrame(const std::string & data);
        std::string extractData(const std::vector<uint8_t> & frame);
        void sendFrame(const FrameCAN & frame);
        FrameCAN receiveFrame();
    };

#endif;
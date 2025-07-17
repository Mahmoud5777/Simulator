#ifndef ECUSENDER_H
    #define ECUSENDER_H
    #include "Ecu.hpp"

    class EcuSender : public Ecu {
        public:
            EcuSender(const std::string& id) : Ecu(id) {}
            void run () override;
    };

#endif
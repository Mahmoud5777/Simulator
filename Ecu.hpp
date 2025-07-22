#ifndef ECU_H
    #define ECU_H
    #include <string>
    #include <iostream>
    class Ecu {
        private:
            std::string ECUID;
        public:
            Ecu(const std::string& id) : ECUID(id) {}
            virtual void run() = 0;
            virtual ~Ecu() = default;
            std::string getECUID() const { return ECUID; }
    };

#endif

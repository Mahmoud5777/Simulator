#ifndef ECU_H
#define ECU_H
#include <string>
class Ecu {
    private:
        std ::string ECUName;
        std::string ECUID;
    public:
        Ecu(const std::string& name, const std::string& id) : ECUName(name), ECUID(id) {}
        virtual std::string reciveData() = 0;
        virtual bool sendData(const std::string& data) = 0;
        virtual ~Ecu() = default;
        std::string getECUID() const { return ECUID; }
};
#endif 
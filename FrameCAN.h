# ifndef FRAMECAN_H
# define FRAMECAN_H
#include <string>

class FrameCAN {
    private:
        int frameID;
        std::string data;
    public:
        FrameCAN(int id, const std::string& data) : frameID(id), data(data) {}
        
        int getFrameID() const { return frameID; }
        std::string getData() const { return data; }
        
        void setData(const std::string& newData) { data = newData; }
        
        // Additional methods can be added as needed
};
# endif // FRAMECAN_H
#ifndef FRAMECAN_H
    #define FRAMECAN_H
    #include <vector>
    #include <cstdint>

    class FrameCAN {
        private:
            uint32_t frameID;
            std::vector<uint8_t> data;
        public:
            FrameCAN() : frameID(0), data() {}        
            FrameCAN(int id, const std::vector<uint8_t>& data) : frameID(id), data(data) {}
            int getFrameID() const { return frameID; }
            std::vector<uint8_t> getData() const { return data; }
            void setData(const std::vector<uint8_t>& newData) { data = newData; }
            
            
    };
#endif
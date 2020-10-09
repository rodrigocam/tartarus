
class LCD {
    public:
        LCD(unsigned char addr);
        ~LCD();
        void write(float internal_temperature, float ambient_temperature, float reference_temperature);

    private:
        unsigned char device_addr;
        int file_descriptor;
};



class Relay {
    public:
        Relay(int pin);
        ~Relay();
        void turn_on();
        void turn_off();
        void _switch();

    private:
        bool on;
        int pin;
};

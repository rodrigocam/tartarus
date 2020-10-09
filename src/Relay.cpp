#include <wiringPi.h>

#include <Relay.h>

Relay::Relay(int n_pin) {
    pin = n_pin;
    pinMode(pin, OUTPUT);
    Relay::turn_off();
}

Relay::~Relay(){
    Relay::turn_off();
}

void Relay::turn_on() {
    digitalWrite(pin, LOW);
    on = true;
}

void Relay::turn_off() {
    digitalWrite(pin, HIGH);
    on = false;
}

void Relay::_switch() {
    if(on) {
        Relay::turn_off();
    } else {
        Relay::turn_on();
    }
}

char* Relay::status() {
    if(on) {
        return "On";
    } else {
        return "Off";
    }
}

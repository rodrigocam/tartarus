#include <wiringPi.h>

#include <Relay.h>

Relay::Relay(int n_pin) {
    pin = n_pin;
    pinMode(pin, OUTPUT);
    on = false;
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

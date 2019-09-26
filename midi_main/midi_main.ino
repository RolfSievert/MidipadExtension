#define CONTROL_STATUS 0b1011

uint_4 channel = 0;
// index 0-7 are soft buttons, 8-9 are touch
uint button_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11}

// A button can not be activated more than every millis_eps milli second
uint millis_eps = 10;
// Saves button timers
uint button_timers[button_pins.size()] = {0};

void setChannel(uint_4 ch) { channel = ch; }

void setup() {
    /*
    Will have 8 soft buttons
    and 2-3 touch buttons
    */

    // Setup buttons as inputs
    for (pin : button_pins) {
        pinMode(pin, INPUT);
    }
    // Set MIDI baud rate
    Serial.begin(31250);
}

void midiControlSend(uint_7 controller_number, bit val) {
    // 1011 CCCC 0NNN NNNN 0VVV VVVV
    // Send control message: C is channel, N controller number (recoomend
    // 70-120), V value
    // Send status byte
    Serial.write((CONTROL_STATUS*16) + channel);
    // Send controller number
    Serial.write(controller_number);
    // Send value (HIGH/LOW)
    Serial.write(val ? 127 : 0);
}

long uint last_time = 0;
void loop() {
    auto dt = millis() - last_time;
    last_time = millis();

    // Decrease time of button
    for (auto &t : button_timers) {
        if (t > 0) {
            t = max(0, dt);
        }
    }

    // Check pin activation
    for (uint i = 0; i < button_pins.size(); i++) {
        if (digitalRead(button_pins[i])) {
            if (button_timers[i] == 0) {
                midiControlSend(i, 1);
            }
            button_timers[i] = millis_eps;
        }
    }
}

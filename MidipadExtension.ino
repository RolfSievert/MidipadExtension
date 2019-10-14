#include <MIDI.h>

/////////////////////////////
#include <MIDI.h>

#if defined(USBCON)
#include <midi_UsbTransport.h>

static const unsigned sUsbTransportBufferSize = 16;
typedef midi::UsbTransport<sUsbTransportBufferSize> UsbTransport;

UsbTransport sUsbTransport;

MIDI_CREATE_INSTANCE(UsbTransport, sUsbTransport, MIDI);

#else // No USB available, fallback to Serial
MIDI_CREATE_DEFAULT_INSTANCE();
#endif
////////////////////////////

#define CONTROL_STATUS 0b1011

int channel = 0;
// index 0-7 are soft buttons, 8-10 are touch
int button_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
int led_pins[] = {14, 15, 16, 17, 18, 19, 20, 21};

int CONTROLLER_BASE_NUMBER = 52;
// A button can not be activated more than every micross_eps micro second
int micros_eps = 10000;
// Saves button timers
int button_timers[sizeof(button_pins) / sizeof(button_pins[0])] = {0};

void setChannel(int ch) { channel = ch; }

void ledOn(int led) { digitalWrite(led_pins[led], HIGH); }

void ledOff(int led) { digitalWrite(led_pins[led], LOW); }

//MIDI_CREATE_INSTANCE(HardwareSerial,Serial, MIDI);


void setup() {
    /// Will have 8 soft buttons
    /// 2-3 touch buttons

    Serial.begin(31250);
    //Serial.begin(115200);
    while (!Serial);

    // Launch MIDI and listen all channels
    //MIDI.begin(MIDI_CHANNEL_OMNI); 
    MIDI.begin();
    // Listen to all incoming messages
    MIDI.begin(MIDI_CHANNEL_OMNI);
    // Setup buttons as inputs
    for (auto &pin : button_pins) {
        pinMode(pin, INPUT);
    }
    for (auto &pin : led_pins) {
        pinMode(pin, OUTPUT);
    }
    // Set MIDI baud rate
    MIDI.sendControlChange(0, 0, 1);
}

void midiControlSend(int button_index, bool val) {
    MIDI.sendControlChange(CONTROLLER_BASE_NUMBER + button_index, val ? 127 : 0, channel);
}

long long int last_time = 0;
void loop() {
    auto dt = micros() - last_time;
    last_time = micros();

    // Check pin activation
    bool pressed_any = false;
    for (int i = 0; i < sizeof(button_pins) / sizeof(button_pins[0]); i++) {
        if (digitalRead(button_pins[i]) == HIGH) {
            if (button_timers[i] == 0) {
                midiControlSend(i, 1);
                ledOn(i);
            }
            // digitalWrite(LED_BUILTIN, HIGH);
            pressed_any = true;
            button_timers[i] = micros_eps;
        } else if (button_timers[i] > 0) {
            button_timers[i] = max(0, button_timers[i] - dt);
            if (button_timers[i] == 0) {
                midiControlSend(i, 0);
                ledOff(i);
            }
        }
    }
}

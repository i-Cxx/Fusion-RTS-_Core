#ifndef BLINK_HPP
#define BLINK_HPP

#include "pico/stdlib.h" // For GPIO functions and sleep_ms

class Blink {
public:
    Blink(uint gpioPin);
    void init();
    void toggle();
    void blink_n_times(int count, unsigned long delay_ms);

    // NEW: Method to get the current state of the LED
    bool is_on() const { return _currentState; } // inline implementation for simplicity

private:
    uint _gpioPin;
    bool _currentState;
};

#endif // BLINK_HPP
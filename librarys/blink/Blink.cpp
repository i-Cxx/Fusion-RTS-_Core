#include "Blink.hpp"
#include "pico/stdlib.h" // Includes SDK functions like gpio_init, gpio_set_dir, gpio_put, sleep_ms

Blink::Blink(uint gpioPin) :
    _gpioPin(gpioPin),
    _currentState(false) { // Initialize with LED off state
    // Constructor just sets up the pin number
}

void Blink::init() {
    gpio_init(_gpioPin);
    gpio_set_dir(_gpioPin, GPIO_OUT);
    gpio_put(_gpioPin, _currentState); // Ensure LED is off initially
}

void Blink::toggle() {
    _currentState = !_currentState;
    gpio_put(_gpioPin, _currentState);
}

void Blink::blink_n_times(int count, unsigned long delay_ms) {
    for (int i = 0; i < count; ++i) {
        toggle(); // Turn LED on (or off)
        sleep_ms(delay_ms);
        toggle(); // Turn LED off (or on)
        sleep_ms(delay_ms);
    }
    // After 'n' blinks, the LED will be in the same state it was before the function call.
    // If you always want it OFF at the end, you could add:
    // if (_currentState) {
    //     toggle();
    // }
}
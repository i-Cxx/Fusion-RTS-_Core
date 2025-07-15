#include "Blink.hpp"
#include "FreeRTOS.h" // For FreeRTOS types like TaskHandle_t
#include "task.h"     // For vTaskDelay, etc.
#include "stdio.h"    // For printf

// Removed WRAPPER_LED_PIN as the pin will now be passed via pvParameters.
// #define WRAPPER_LED_PIN PICO_DEFAULT_LED_PIN

// This is the actual FreeRTOS task function.
// It's declared as extern "C" so the C linker can find it.
extern "C" void vBlinkTaskCpp(void *pvParameters) {
    // Retrieve the GPIO pin from pvParameters
    // pvParameters is a void* pointer. We need to cast it to an integer type.
    uint gpio_pin = (uintptr_t)pvParameters; // Use uintptr_t for robust conversion

    // Create an instance of the C++ Blink class with the passed pin
    // This object will live for the duration of the task.
    Blink led(gpio_pin); // Use the pin passed as parameter

    // Initialize the LED pin using the Blink class method
    led.init();
    printf("C++ Blink task initialized LED on pin %d.\n", gpio_pin); // Adjust print message

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(500); // 500ms blink frequency

    for (;;) {
        led.toggle(); // Toggle the LED state
        // Use the new is_on() method for output
        printf("LED on Pin %d is %s! (via C++ Blink class)\n", gpio_pin, led.is_on() ? "on" : "off");

        // Wait for the next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
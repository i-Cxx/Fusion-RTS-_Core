#include "Blink.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

// Entferne die globale Definition von WRAPPER_LED_PIN hier,
// da der Pin über pvParameters übergeben wird.
// #define WRAPPER_LED_PIN PICO_DEFAULT_LED_PIN

extern "C" void vBlinkTaskCpp(void *pvParameters) {
    // Hole den GPIO-Pin aus den pvParameters
    // Wichtig: pvParameters ist ein void*, daher casten wir zu uint32_t,
    // um sicherzustellen, dass die Größe stimmt, und dann zu uint.
    uint gpio_pin = (uint32_t)pvParameters;

    // Erstelle eine Instanz der C++ Blink-Klasse mit dem übergebenen Pin
    Blink led(gpio_pin);

    // Initialisiere den LED-Pin
    led.init();
    printf("C++ Blink task initialized LED on pin %d.\n", gpio_pin);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(500); // 500ms Blinkfrequenz

    for (;;) {
        led.toggle(); // Schalte die LED um
        // Verwende die neue is_on() Methode für die Ausgabe
        printf("LED an Pin %d ist %s! (via C++ Blink class)\n", gpio_pin, led.is_on() ? "an" : "aus");

        // Warte auf den nächsten Zyklus
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
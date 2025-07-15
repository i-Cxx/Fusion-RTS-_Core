#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"   // Required for I2C initialization
#include "hardware/gpio.h"  // Required for GPIO functions

// Include the LCD 1602 driver header
#include "lcd_1602_i2c.h"

// Include the SSD1306 OLED driver header
#include "ssd1306_i2c.h"


// --- IMPORTANT: Declare the C++ FreeRTOS Task Function as extern "C" ---
// In a C file, this extern "C" block is needed if the function
// is defined in a C++ file and has C linkage.
#ifdef __cplusplus
extern "C" {
#endif
extern void vBlinkTaskCpp(void *pvParameters);
#ifdef __cplusplus
}
#endif


// --- LED Task Configuration ---
// These are the specific GPIO pins you want to use
#define MY_CUSTOM_LED_PIN 25 // Typically the onboard LED on the Pico
#define WARN_LED 16          // Your specific GPIO16 for the external LED

#define BLINK_TASK_PRIORITY (tskIDLE_PRIORITY + 1)


// --- LCD 1602 Task Configuration ---
#define LCD1602_TASK_PRIORITY (tskIDLE_PRIORITY + 2) // Higher priority than LED task
#define LCD1602_I2C_INSTANCE    i2c0    // Using i2c0 for both displays for simplicity
#define LCD1602_I2C_SDA_PIN     4       // GPIO pin for I2C SDA (e.g., GP4)
#define LCD1602_I2C_SCL_PIN     5       // GPIO pin for I2C SCL (e.g., GP5)
#define LCD1602_I2C_ADDR        0x27    // Standard I2C address for PCF8574-based LCDs (can be 0x3F)

// Global instance of the LCD 1602 structure
lcd_1602_i2c_t my_lcd1602;


// --- SSD1306 OLED Task Configuration ---
#define SSD1306_TASK_PRIORITY (tskIDLE_PRIORITY + 3) // Even higher priority
#define SSD1306_I2C_INSTANCE    i2c0    // Using the same i2c0 instance
#define SSD1306_I2C_SDA_PIN     4       // Same SDA pin as LCD1602
#define SSD1306_I2C_SCL_PIN     5       // Same SCL pin as LCD1602
#define SSD1306_OLED_ADDR       0x3C    // Standard I2C address for SSD1306 (can be 0x3D)


// Frame buffer for the SSD1306 display
uint8_t ssd1306_frame_buffer[SSD1306_BUF_LEN];
struct render_area ssd1306_full_frame_area = {
    .start_col = 0,
    .end_col = SSD1306_WIDTH - 1,
    .start_page = 0,
    .end_page = SSD1306_NUM_PAGES - 1
};


// --- TASKS - FUNCTIONS (Non-LED) ---

// --- Task function to control the LCD 1602 ---
void vLcd1602Task(void *pvParameters) {
    // I2C hardware is initialized once in main().
    // Only initialize the LCD driver here.
    lcd_init(&my_lcd1602, LCD1602_I2C_INSTANCE, LCD1602_I2C_ADDR);
    printf("LCD 1602 initialization completed in task.\n");

    lcd_clear(&my_lcd1602);
    lcd_set_cursor(&my_lcd1602, 0, 0);
    lcd_write_string(&my_lcd1602, "Console > ");
    lcd_set_cursor(&my_lcd1602, 1, 0);
    lcd_write_string(&my_lcd1602, "Started");

    for (;;) {
        static int dot_state = 0;
        lcd_set_cursor(&my_lcd1602, 1, 15); // Last column of the second row
        if (dot_state == 0) {
            lcd_write_char(&my_lcd1602, '.');
            dot_state = 1;
        } else {
            lcd_write_char(&my_lcd1602, ' ');
            dot_state = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


// --- Task function to control the SSD1306 OLED ---
void vSsd1306Task(void *pvParameters) {
    // I2C hardware is initialized once in main().
    // Only initialize the SSD1306 driver here.
    ssd1306_init(SSD1306_I2C_INSTANCE, SSD1306_I2C_SDA_PIN, SSD1306_I2C_SCL_PIN);
    printf("SSD1306 OLED initialization completed in task.\n");

    // Calculate buffer length for full frame
    ssd1306_calc_render_area_buflen(&ssd1306_full_frame_area);

    // Clear the frame buffer and render
    memset(ssd1306_frame_buffer, 0, SSD1306_BUF_LEN);
    ssd1306_render(ssd1306_frame_buffer, &ssd1306_full_frame_area);

    // --- Display the Raspberry Pi image ---
    struct render_area image_area = {
        .start_col = 0, // Start image at column 0
        .end_col = IMG_WIDTH - 1,
        .start_page = 0, // Start image at page 0 (top of display)
        .end_page = (IMG_HEIGHT / SSD1306_PAGE_HEIGHT) - 1
    };
    ssd1306_calc_render_area_buflen(&image_area); // Calculate buflen for the image area

    // Render the raspberry image
    ssd1306_render((uint8_t*)raspberry26x32, &image_area);
    printf("Raspberry image displayed on OLED.\n");

    // Write initial text to OLED (below the image, if space allows, or over it)
    // Assuming 128x64 display: image is 32px high, so text can start at y=32 (page 4)
    if (SSD1306_HEIGHT == 64) {
        ssd1306_write_string(ssd1306_frame_buffer, 0, 32, "BlackzCoreOS"); // Line 4 (y=32)
        ssd1306_write_string(ssd1306_frame_buffer, 0, 40, "Loading...");   // Line 5 (y=40)
    } else { // For 128x32 display, image takes full height, so text might overlap
        ssd1306_write_string(ssd1306_frame_buffer, 0, 0, "BlackzCoreOS"); // Overlap with image
        ssd1306_write_string(ssd1306_frame_buffer, 0, 8, "Loading...");   // Overlap with image
    }
    ssd1306_render(ssd1306_frame_buffer, &ssd1306_full_frame_area);


    // Simple animation loop for "Loading..." text
    int counter = 0;
    for (;;) {
        // Clear the "Loading..." line (assuming it's on page 1 or 5 depending on display height)
        int loading_text_y_offset = (SSD1306_HEIGHT == 64) ? 40 : 8;
        // This memset might clear more than just the text, be careful.
        // It clears a whole row/page-width based on the start_page.
        // Consider a more targeted clear or redraw of the specific text area.
        memset(ssd1306_frame_buffer + (loading_text_y_offset / SSD1306_PAGE_HEIGHT) * SSD1306_WIDTH, 0, SSD1306_WIDTH);
        ssd1306_write_string(ssd1306_frame_buffer, 0, loading_text_y_offset, "Loading...");

        // Add dots to "Loading..."
        for (int i = 0; i < (counter % 4); i++) {
            ssd1306_write_char(ssd1306_frame_buffer, 8 * 9 + i * 8, loading_text_y_offset, '.');
        }
        ssd1306_render(ssd1306_frame_buffer, &ssd1306_full_frame_area);

        counter++;
        vTaskDelay(pdMS_TO_TICKS(250)); // Update every 250ms
    }
}

// --- END OF TASKS - FUNCTIONS ---


// --- Main function of the program ---
int main() {
    stdio_init_all();
    printf("FreeRTOS BlackzCoreOS - LED, LCD 1602 and SSD1306 OLED Example\n");

    // --- I2C Global Initialization (if both displays share the same bus) ---
    // Initialize i2c0 once for both displays.
    // If you use separate I2C instances (e.g., i2c0 for LCD, i2c1 for OLED),
    // you would initialize them separately here.
    i2c_init(LCD1602_I2C_INSTANCE, 100 * 1000); // Initialize i2c0 at 100kHz
    gpio_set_function(LCD1602_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(LCD1602_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(LCD1602_I2C_SDA_PIN);
    gpio_pull_up(LCD1602_I2C_SCL_PIN);
    printf("Shared I2C0 initialized on SDA: %d, SCL: %d\n", LCD1602_I2C_SDA_PIN, LCD1602_I2C_SCL_PIN);


    // Create the LED blink task using the C++ wrapper function
    // We pass the GPIO pin as parameters to the task.
    if (xTaskCreate(vBlinkTaskCpp, "BlinkTask_LED25", configMINIMAL_STACK_SIZE * 3, (void*)MY_CUSTOM_LED_PIN, BLINK_TASK_PRIORITY, NULL) != pdPASS) {
        printf("Error: BlinkTask_LED25 could not be created!\n");
        while (1) {}
    }

    if (xTaskCreate(vBlinkTaskCpp, "BlinkTask_WARNLED16", configMINIMAL_STACK_SIZE * 3, (void*)WARN_LED, BLINK_TASK_PRIORITY, NULL) != pdPASS) {
        printf("Error: BlinkTask_WARNLED16 could not be created!\n");
        while (1) {}
    }


    // Create the LCD 1602 task
    if (xTaskCreate(vLcd1602Task, "Lcd1602Task", configMINIMAL_STACK_SIZE * 6, NULL, LCD1602_TASK_PRIORITY, NULL) != pdPASS) {
        printf("Error: Lcd1602Task could not be created!\n");
        while (1) {}
    }

    // Create the SSD1306 OLED task
    // SSD1306 might need a larger stack due to frame buffer operations and string handling
    if (xTaskCreate(vSsd1306Task, "Ssd1306Task", configMINIMAL_STACK_SIZE * 16, NULL, SSD1306_TASK_PRIORITY, NULL) != pdPASS) {
        printf("Error: Ssd1306Task could not be created!\n");
        while (1) {}
    }

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    printf("Error: Scheduler terminated!\n");
    for (;;) {} // Should never be reached

    return 0;
}
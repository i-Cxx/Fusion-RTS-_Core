#include "lcd_1602_i2c.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

// --- LCD Hardware Konfiguration ---
// These defines can stay here if they are only used within this C file
// or if they are also defined in the header for external use.
// For a library, it's often better to pass these as parameters or have them in the header.
// If these are only needed by the main app, they should be in blink.c
// For now, let's assume they are driver-specific constants.
#define LCD_I2C_INSTANCE    i2c0
#define LCD_I2C_SDA_PIN     4
#define LCD_I2C_SCL_PIN     5
#define LCD_I2C_ADDR        0x27

// REMOVE THIS LINE: lcd_1602_i2c_t my_lcd;
// The 'my_lcd' instance will be defined in blink.c and passed to these functions.


// Hilfsfunktionen für den LCD-Treiber (static) - unverändert
static void i2c_write_byte(lcd_1602_i2c_t *lcd, uint8_t val) {
    i2c_write_blocking(lcd->i2c, lcd->i2c_addr, &val, 1, false);
}

static void lcd_toggle_enable(lcd_1602_i2c_t *lcd, uint8_t val) {
    sleep_us(600);
    i2c_write_byte(lcd, val | LCD_ENABLE_BIT);
    sleep_us(600);
    i2c_write_byte(lcd, val & ~LCD_ENABLE_BIT);
    sleep_us(600);
}

static void lcd_send_byte(lcd_1602_i2c_t *lcd, uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | lcd->backlight;
    uint8_t low = mode | ((val << 4) & 0xF0) | lcd->backlight;

    i2c_write_byte(lcd, high);
    lcd_toggle_enable(lcd, high);
    i2c_write_byte(lcd, low);
    lcd_toggle_enable(lcd, low);
}


// --- LCD Initialisierungs- und Steuerfunktionen (wie gehabt) ---
void lcd_init(lcd_1602_i2c_t *lcd, i2c_inst_t *i2c, uint8_t i2c_addr) {
    lcd->i2c = i2c;
    lcd->i2c_addr = i2c_addr;
    lcd->backlight = LCD_BACKLIGHT;

    sleep_ms(50); // Power on delay

    // Init sequence 4-bit mode
    lcd_send_byte(lcd, 0x03, 0);
    sleep_ms(5);
    lcd_send_byte(lcd, 0x03, 0);
    sleep_us(150);
    lcd_send_byte(lcd, 0x03, 0);
    sleep_us(150);
    lcd_send_byte(lcd, 0x02, 0);

    // Function set: 2 lines, 5x8 dots
    lcd_send_byte(lcd, LCD_FUNCTIONSET | LCD_2LINE, 0);
    // Display on, cursor off, blink off
    lcd_send_byte(lcd, LCD_DISPLAYCONTROL | LCD_DISPLAYON, 0);
    // Clear display
    lcd_send_byte(lcd, LCD_CLEARDISPLAY, 0);
    sleep_ms(2); // Clear display needs 2ms
    // Entry mode set: left to right
    lcd_send_byte(lcd, LCD_ENTRYMODESET | LCD_ENTRYLEFT, 0);
}

void lcd_clear(lcd_1602_i2c_t *lcd) {
    lcd_send_byte(lcd, LCD_CLEARDISPLAY, 0);
    sleep_ms(2);
}

void lcd_set_cursor(lcd_1602_i2c_t *lcd, uint8_t line, uint8_t position) {
    uint8_t line_offsets[] = {0x00, 0x40};
    if(line > 1) line = 1; // max 2 lines
    lcd_send_byte(lcd, LCD_SETDDRAMADDR | (position + line_offsets[line]), 0);
}

void lcd_write_char(lcd_1602_i2c_t *lcd, char c) {
    lcd_send_byte(lcd, (uint8_t)c, 1);
}

void lcd_write_string(lcd_1602_i2c_t *lcd, const char* str) {
    while(*str) {
        lcd_write_char(lcd, *str++);
    }
}

// REMOVE ALL FREE RTOS TASKS AND MAIN FUNCTION FROM HERE.
// They belong in blink.c
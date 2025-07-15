#ifndef LCD_1602_I2C_H
#define LCD_1602_I2C_H

#include <stdint.h>
#include "hardware/i2c.h"

// LCD Kommandos (optional als defines)
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME   0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET  0x20
#define LCD_SETDDRAMADDR 0x80

// Flags für display control
#define LCD_DISPLAYON  0x04
#define LCD_ENTRYLEFT  0x02
#define LCD_2LINE      0x08
#define LCD_BACKLIGHT  0x08
#define LCD_ENABLE_BIT 0x04

typedef struct {
    i2c_inst_t *i2c;      // z.B. i2c0 oder i2c1
    uint8_t i2c_addr;     // meist 0x27 oder 0x3F
    uint8_t backlight;    // LCD_BACKLIGHT oder 0x00
} lcd_1602_i2c_t;

// Initialisierung mit i2c Instanz und Adresse
void lcd_init(lcd_1602_i2c_t *lcd, i2c_inst_t *i2c, uint8_t i2c_addr);

// LCD Funktionen
void lcd_clear(lcd_1602_i2c_t *lcd);
void lcd_set_cursor(lcd_1602_i2c_t *lcd, uint8_t line, uint8_t position);
void lcd_write_char(lcd_1602_i2c_t *lcd, char c);
void lcd_write_string(lcd_1602_i2c_t *lcd, const char* str);

#endif // LCD_1602_I2C_H

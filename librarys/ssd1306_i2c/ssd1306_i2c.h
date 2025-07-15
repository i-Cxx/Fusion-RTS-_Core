#ifndef SSD1306_I2C_H
#define SSD1306_I2C_H

#include "pico/stdlib.h"    // For bool, uint8_t, _u macro, sleep_ms/us
#include "hardware/i2c.h"   // For i2c_inst_t
#include <string.h>         // For memcpy (used in SSD1306_send_buf)
#include <stdlib.h>         // For malloc/free (used in SSD1306_send_buf)
#include <ctype.h>          // For toupper (used in WriteChar)
#include <assert.h>         // For assert (used in SetPixel)

// Include external assets (assuming they are in the same directory or accessible via include paths)
// These files must define 'font' and 'raspberry26x32' arrays, and IMG_WIDTH/IMG_HEIGHT
#include "raspberry26x32.h" // Example: defines raspberry26x32, IMG_WIDTH, IMG_HEIGHT
#include "ssd1306_font.h"   // Example: defines font array

// Define the size of the display. Adjust these based on your specific OLED.
// REMOVE THE DUPLICATE DEFINITION. Keep only the desired one.
#define SSD1306_HEIGHT              64  // This is the desired height
#define SSD1306_WIDTH               128 // This is the desired width

// Default I2C address for SSD1306 (can be 0x3C or 0x3D)
#define SSD1306_I2C_ADDR            _u(0x3C)

// Default I2C clock frequency in kHz (400 is common, up to 1000 can work)
#define SSD1306_I2C_CLK_KHZ         400

// Commands (see datasheet)
#define SSD1306_SET_MEM_MODE        _u(0x20)
#define SSD1306_SET_COL_ADDR        _u(0x21)
#define SSD1306_SET_PAGE_ADDR       _u(0x22)
#define SSD1306_SET_HORIZ_SCROLL    _u(0x26)
#define SSD1306_SET_SCROLL          _u(0x2E)
#define SSD1306_SET_DISP_START_LINE _u(0x40)
#define SSD1306_SET_CONTRAST        _u(0x81)
#define SSD1306_SET_CHARGE_PUMP     _u(0x8D)
#define SSD1306_SET_SEG_REMAP       _u(0xA0)
#define SSD1306_SET_ENTIRE_ON       _u(0xA4)
#define SSD1306_SET_ALL_ON          _u(0xA5)
#define SSD1306_SET_NORM_DISP       _u(0xA6)
#define SSD1306_SET_INV_DISP        _u(0xA7)
#define SSD1306_SET_MUX_RATIO       _u(0xA8)
#define SSD1306_SET_DISP            _u(0xAE)
#define SSD1306_SET_COM_OUT_DIR     _u(0xC0)
#define SSD1306_SET_COM_OUT_DIR_FLIP _u(0xC0) // Same as 0xC0, just for clarity
#define SSD1306_SET_DISP_OFFSET     _u(0xD3)
#define SSD1306_SET_DISP_CLK_DIV    _u(0xD5)
#define SSD1306_SET_PRECHARGE       _u(0xD9)
#define SSD1306_SET_COM_PIN_CFG     _u(0xDA)
#define SSD1306_SET_VCOM_DESEL      _u(0xDB)

#define SSD1306_PAGE_HEIGHT         _u(8)
#define SSD1306_NUM_PAGES           (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
#define SSD1306_BUF_LEN             (SSD1306_NUM_PAGES * SSD1306_WIDTH)

#define SSD1306_WRITE_MODE          _u(0xFE)
#define SSD1306_READ_MODE           _u(0xFF)

/**
 * @brief Structure to define a rectangular area for rendering on the SSD1306.
 */
struct render_area {
    uint8_t start_col;  ///< Starting column (X-coordinate)
    uint8_t end_col;    ///< Ending column (X-coordinate)
    uint8_t start_page; ///< Starting page (Y-coordinate, 1 page = 8 pixels)
    uint8_t end_page;   ///< Ending page (Y-coordinate)
    int buflen;         ///< Calculated buffer length for this area
};

// Public API Functions
// Note: i2c_default is used internally by these functions.
// For more flexibility, one could pass i2c_inst_t* and pin numbers to SSD1306_init.

/**
 * @brief Calculates the buffer length required for a given render area.
 * @param area Pointer to the render_area structure.
 */
void ssd1306_calc_render_area_buflen(struct render_area *area);

/**
 * @brief Sends a single command byte to the SSD1306 display.
 * @param cmd The command byte to send.
 */
void ssd1306_send_cmd(uint8_t cmd);

/**
 * @brief Sends a list of command bytes to the SSD1306 display.
 * @param buf Pointer to the array of command bytes.
 * @param num Number of command bytes in the array.
 */
void ssd1306_send_cmd_list(uint8_t *buf, int num);

/**
 * @brief Sends a buffer of pixel data to the SSD1306 display.
 * Assumes horizontal addressing mode.
 * @param buf Pointer to the pixel data buffer.
 * @param buflen Length of the pixel data buffer.
 */
void ssd1306_send_buf(uint8_t buf[], int buflen);

/**
 * @brief Initializes the SSD1306 OLED display.
 * This function also initializes the I2C peripheral and GPIOs.
 * @param i2c_instance The I2C instance to use (e.g., i2c0).
 * @param sda_pin The GPIO pin for I2C SDA.
 * @param scl_pin The GPIO pin for I2C SCL.
 */
void ssd1306_init(i2c_inst_t *i2c_instance, uint gpio_sda, uint gpio_scl);

/**
 * @brief Activates or deactivates horizontal scrolling on the display.
 * @param on True to activate scrolling, false to deactivate.
 */
void ssd1306_scroll(bool on);

/**
 * @brief Renders a buffer of pixel data to a specified area on the display.
 * @param buf Pointer to the pixel data buffer.
 * @param area Pointer to the render_area structure defining the target region.
 */
void ssd1306_render(uint8_t *buf, struct render_area *area);

/**
 * @brief Sets or clears a single pixel in the frame buffer.
 * @param buf Pointer to the frame buffer.
 * @param x X-coordinate of the pixel.
 * @param y Y-coordinate of the pixel.
 * @param on True to set (turn on) the pixel, false to clear (turn off).
 */
void ssd1306_set_pixel(uint8_t *buf, int x, int y, bool on);

/**
 * @brief Draws a line in the frame buffer using Bresenham's algorithm.
 * @param buf Pointer to the frame buffer.
 * @param x0 Start X-coordinate.
 * @param y0 Start Y-coordinate.
 * @param x1 End X-coordinate.
 * @param y1 End Y-coordinate.
 * @param on True to draw (set pixels), false to erase (clear pixels).
 */
void ssd1306_draw_line(uint8_t *buf, int x0, int y0, int x1, int y1, bool on);

/**
 * @brief Writes a single character to the frame buffer.
 * @param buf Pointer to the frame buffer.
 * @param x X-coordinate for the character's top-left corner.
 * @param y Y-coordinate for the character's top-left corner.
 * @param ch The character to write.
 */
void ssd1306_write_char(uint8_t *buf, int16_t x, int16_t y, uint8_t ch);

/**
 * @brief Writes a string to the frame buffer.
 * @param buf Pointer to the frame buffer.
 * @param x Start X-coordinate for the string.
 * @param y Start Y-coordinate for the string.
 * @param str The string to write.
 */
void ssd1306_write_string(uint8_t *buf, int16_t x, int16_t y, char *str);

#endif // SSD1306_I2C_H
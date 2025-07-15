/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ssd1306_i2c.h" // Include the new header file
// Remove stdio.h, string.h, stdlib.h, ctype.h as they are now included via ssd1306_i2c.h
// Remove pico/binary_info.h as bi_decl is removed
// Remove hardware/i2c.h as it's included via ssd1306_i2c.h

// REMOVE THESE LINES:
// #define SSD1306_HEIGHT              64
// #define SSD1306_WIDTH               128

#define SSD1306_I2C_ADDR            _u(0x3C) // This can stay here or be moved to header if needed externally

// commands (see datasheet)
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
#define SSD1306_SET_COM_OUT_DIR_FLIP _u(0xC0)

#define SSD1306_SET_DISP_OFFSET     _u(0xD3)
#define SSD1306_SET_DISP_CLK_DIV    _u(0xD5)
#define SSD1306_SET_PRECHARGE       _u(0xD9)
#define SSD1306_SET_COM_PIN_CFG     _u(0xDA)
#define SSD1306_SET_VCOM_DESEL      _u(0xDB)

#define SSD1306_PAGE_HEIGHT         _u(8)
#define SSD1306_NUM_PAGES           (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT) // SSD1306_HEIGHT now comes from header
#define SSD1306_BUF_LEN             (SSD1306_NUM_PAGES * SSD1306_WIDTH)    // SSD1306_WIDTH now comes from header

#define SSD1306_WRITE_MODE          _u(0xFE)
#define SSD1306_READ_MODE           _u(0xFF)

// Internal static variables for I2C instance and pins
static i2c_inst_t *ssd1306_i2c_instance = NULL;
static uint ssd1306_sda_pin = 0;
static uint ssd1306_scl_pin = 0;

// Static variables for font cache (these are internal to this .c file)
static uint8_t reversed[sizeof(font)] = {0}; // 'font' must be defined in ssd1306_font.h

// Static helper functions for font rendering
static inline int GetFontIndex(uint8_t ch) {
    if (ch >= 'A' && ch <='Z') {
        return  ch - 'A' + 1;
    }
    else if (ch >= '0' && ch <='9') {
        return  ch - '0' + 27;
    }
    else return  0; // Not got that char so space.
}

static uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}
static void FillReversedCache() {
    if (reversed[0] == 0) { // Only fill if not already filled
        for (int i=0;i<sizeof(font);i++)
            reversed[i] = reverse(font[i]);
    }
}

// Public API Functions (renamed to avoid conflicts and clearly mark as library functions)
void ssd1306_calc_render_area_buflen(struct render_area *area) {
    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

void ssd1306_send_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(ssd1306_i2c_instance, SSD1306_I2C_ADDR, buf, 2, false);
}

void ssd1306_send_cmd_list(uint8_t *buf, int num) {
    for (int i=0;i<num;i++)
        ssd1306_send_cmd(buf[i]);
}

void ssd1306_send_buf(uint8_t buf[], int buflen) {
    uint8_t *temp_buf = malloc(buflen + 1);
    if (temp_buf == NULL) {
        // Handle memory allocation error
        return;
    }
    temp_buf[0] = 0x40;
    memcpy(temp_buf+1, buf, buflen);
    i2c_write_blocking(ssd1306_i2c_instance, SSD1306_I2C_ADDR, temp_buf, buflen + 1, false);
    free(temp_buf);
}

void ssd1306_init(i2c_inst_t *i2c_instance, uint gpio_sda, uint gpio_scl) {
    ssd1306_i2c_instance = i2c_instance;
    ssd1306_sda_pin = gpio_sda;
    ssd1306_scl_pin = gpio_scl;

    i2c_init(ssd1306_i2c_instance, SSD1306_I2C_CLK_KHZ * 1000);
    gpio_set_function(ssd1306_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(ssd1306_scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(ssd1306_sda_pin);
    gpio_pull_up(ssd1306_scl_pin);

    uint8_t cmds[] = {
        SSD1306_SET_DISP,
        SSD1306_SET_MEM_MODE, 0x00,
        SSD1306_SET_DISP_START_LINE,
        SSD1306_SET_SEG_REMAP | 0x01,
        SSD1306_SET_MUX_RATIO, SSD1306_HEIGHT - 1,
        SSD1306_SET_COM_OUT_DIR | 0x08,
        SSD1306_SET_DISP_OFFSET, 0x00,
        SSD1306_SET_COM_PIN_CFG,
#if ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 32))
        0x02,
#elif ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 64))
        0x12,
#else
        0x02, // Default for other sizes, might need adjustment
#endif
        SSD1306_SET_DISP_CLK_DIV, 0x80,
        SSD1306_SET_PRECHARGE, 0xF1,
        SSD1306_SET_VCOM_DESEL, 0x30,
        SSD1306_SET_CONTRAST, 0xFF,
        SSD1306_SET_ENTIRE_ON,
        SSD1306_SET_NORM_DISP,
        SSD1306_SET_CHARGE_PUMP, 0x14,
        SSD1306_SET_SCROLL | 0x00,
        SSD1306_SET_DISP | 0x01,
    };
    ssd1306_send_cmd_list(cmds, count_of(cmds));
}

void ssd1306_scroll(bool on) {
    uint8_t cmds[] = {
        SSD1306_SET_HORIZ_SCROLL | 0x00,
        0x00, 0x00, 0x00,
        0x03, // end page 3 (for 32px height, this is page 3) -- for 64px, it should be 7
        0x00, 0xFF,
        SSD1306_SET_SCROLL | (on ? 0x01 : 0)
    };
    // Adjust end page for 64-height display if needed
    if (SSD1306_HEIGHT == 64) {
        cmds[4] = SSD1306_NUM_PAGES - 1; // End page is last page index
    }
    ssd1306_send_cmd_list(cmds, count_of(cmds));
}

void ssd1306_render(uint8_t *buf, struct render_area *area) {
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR, area->start_col, area->end_col,
        SSD1306_SET_PAGE_ADDR, area->start_page, area->end_page
    };
    ssd1306_send_cmd_list(cmds, count_of(cmds));
    ssd1306_send_buf(buf, area->buflen);
}

void ssd1306_set_pixel(uint8_t *buf, int x, int y, bool on) {
    assert(x >= 0 && x < SSD1306_WIDTH && y >=0 && y < SSD1306_HEIGHT);
    const int BytesPerRow = SSD1306_WIDTH;
    int byte_idx = (y / 8) * BytesPerRow + x;
    uint8_t byte = buf[byte_idx];
    if (on) byte |=  1 << (y % 8);
    else byte &= ~(1 << (y % 8));
    buf[byte_idx] = byte;
}

void ssd1306_draw_line(uint8_t *buf, int x0, int y0, int x1, int y1, bool on) {
    int dx =  abs(x1-x0); int sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0); int sy = y0<y1 ? 1 : -1;
    int err = dx+dy; int e2;
    while (true) {
        ssd1306_set_pixel(buf, x0, y0, on);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void ssd1306_write_char(uint8_t *buf, int16_t x, int16_t y, uint8_t ch) {
    FillReversedCache(); // Ensure cache is filled

    if (x > SSD1306_WIDTH - 8 || y > SSD1306_HEIGHT - 8)
        return;

    y = y/8; // For the moment, only write on Y row boundaries (every 8 vertical pixels)

    ch = toupper(ch);
    int idx = GetFontIndex(ch);
    int fb_idx = y * SSD1306_WIDTH + x;

    for (int i=0;i<8;i++) {
        buf[fb_idx++] = reversed[idx * 8 + i];
    }
}

void ssd1306_write_string(uint8_t *buf, int16_t x, int16_t y, char *str) {
    if (x > SSD1306_WIDTH - 8 || y > SSD1306_HEIGHT - 8)
        return;

    while (*str) {
        ssd1306_write_char(buf, x, y, *str++);
        x+=8;
    }
}
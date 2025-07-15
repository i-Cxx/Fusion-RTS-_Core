# Definition einer statischen Bibliothek für den SSD1306-Treiber.
# Der Name 'ssd1306_i2c_lib' ist das CMake-Target, mit dem andere Projekte verknüpfen.
add_library(ssd1306_i2c_lib STATIC
    ssd1306_i2c.c
    # Wenn 'ssd1306_font.c' existieren und kompiliert werden muss, fügen Sie es hier hinzu.
    # Wenn 'ssd1306_font.h' nur ein Header ist, ist es hier nicht nötig,
    # da es über das include-Verzeichnis gefunden wird.
)

# Setzt das aktuelle Verzeichnis als öffentliches Include-Verzeichnis.
target_include_directories(ssd1306_i2c_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    # Falls raspberry26x32.h hierher verschoben wurde und es sich um einen Header handelt,
    # ist der obige Pfad ausreichend, da er das aktuelle Verzeichnis abdeckt.
)

# Listet die Abhängigkeiten des SSD1306-Treibers auf.
target_link_libraries(ssd1306_i2c_lib
    pico_stdlib
    hardware_i2c  # Da der SSD1306-Treiber I2C verwendet
    hardware_gpio # Falls GPIOs direkt genutzt werden
)
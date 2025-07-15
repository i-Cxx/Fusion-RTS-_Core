# Definition einer statischen Bibliothek für den LCD-Treiber.
# Der Name 'lcd_1602_i2c_lib' ist das CMake-Target, mit dem andere Projekte verknüpfen.
add_library(lcd_1602_i2c_lib STATIC
    lcd_1602_i2c.c
)

# Setzt das aktuelle Verzeichnis als öffentliches Include-Verzeichnis.
# Das bedeutet, jedes Projekt, das 'lcd_1602_i2c_lib' verknüpft,
# findet automatisch die Header-Dateien in diesem Ordner.
target_include_directories(lcd_1602_i2c_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Listet die Abhängigkeiten der LCD-Bibliothek auf.
# Dies stellt sicher, dass die benötigten Pico SDK-Komponenten kompiliert und verknüpft werden.
target_link_libraries(lcd_1602_i2c_lib
    pico_stdlib
    hardware_i2c  # Da der LCD-Treiber I2C verwendet
    hardware_gpio # Falls GPIOs direkt genutzt werden (oft für I2C)
)
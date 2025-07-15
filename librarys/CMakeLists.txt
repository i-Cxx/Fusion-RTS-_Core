# Dies ist die CMakeLists.txt für den übergeordneten 'librarys'-Ordner.
# Sie bindet alle einzelnen Bibliotheken in diesem Verzeichnis ein.

# Binde die LCD-Bibliothek ein
add_subdirectory(lcd_1602_i2c)

# Binde die SSD1306-Bibliothek ein
add_subdirectory(ssd1306_i2c)

# Binde Blink Biliothek ein
add_subdirectory(blink)


# Fügen Sie hier weitere Bibliotheken hinzu, die Sie später in diesem Ordner erstellen
# add_subdirectory(name_der_neuen_bibliothek)

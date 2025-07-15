# CMakeLists.txt für die Blink-Bibliothek
add_library(blink_lib STATIC
    Blink.cpp
    blink_task_wrapper.cpp
)

target_include_directories(blink_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(blink_lib
    pico_stdlib
    hardware_gpio
    freertos          # Wichtig: blink_task_wrapper.cpp nutzt FreeRTOS-Funktionen
    # hardware_i2c      # Optional: Kann entfernt werden, da Blink-Code kein I2C direkt nutzt
)
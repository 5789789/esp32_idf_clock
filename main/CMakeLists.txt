# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c" "console.c" "oled.c" "u8g2_esp32_hal.c" "cmd_wifi.c" "time_driver.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()

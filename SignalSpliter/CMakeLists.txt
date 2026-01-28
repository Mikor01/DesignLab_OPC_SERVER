cmake_minimum_required(VERSION 3.25)

# Select board:
# set(PICO_BOARD pico_w)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(main C CXX ASM)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

pico_sdk_init()

# Uncomment to set different 
# add_compile_definitions(SYS_CLK_MHZ=200)


add_executable(${PROJECT_NAME}
    main.c
    LED/led.c
    SPI/spi.c
    Display/display.c
    Display/draw.c
    Display/draw_dma.c
    Interface/interface.c
    Encoder/encoder.c
    I2C/i2c.c
    Expander/expander.c
    Interface/Screensaver/screensaver.c
)

include_directories(${PROJECT_NAME}
    .
    LED
    SPI
    Display
    Display/Font
    Interface
    Encoder
    I2C
    Expander
    Interface/Screensaver
)

# pico_generate_pio_header(${PROJECT_NAME}
# )

pico_add_extra_outputs(${PROJECT_NAME})


target_link_libraries(${PROJECT_NAME}
    pico_async_context_freertos
    FreeRTOS-Kernel-Heap4

    pico_stdlib
    hardware_gpio
    hardware_spi
    hardware_dma
    hardware_i2c
    pico_stdio_uart
)

pico_enable_stdio_usb(${PROJECT_NAME}  1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

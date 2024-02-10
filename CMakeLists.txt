cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(main main.cpp)
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)
pico_add_extra_outputs(main)

include_directories( "lib" ) 
add_subdirectory( lib )

target_include_directories(main PRIVATE ${CMAKE_CURRENT_LIST_DIR} )
target_link_libraries(
    main
    lib
    pico_stdlib 
    hardware_adc
    pico_cyw43_arch_lwip_poll
    pico_lwip_mbedtls
    pico_mbedtls
    hardware_rtc
) 
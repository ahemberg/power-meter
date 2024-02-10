
set(FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/ntp.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/ntp.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/timeutils.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/timeutils.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/measurement.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/measurement.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/influxclient.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/influxclient.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/http_requests.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/http_requests.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tls_common.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tls_common.hpp
)

add_library(lib INTERFACE)
target_include_directories(lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(lib INTERFACE pico_stdlib pico_cyw43_arch_lwip_poll hardware_rtc pico_lwip_mbedtls pico_mbedtls)
target_sources(lib INTERFACE ${FILES})
# driver/CMakeLists.txt
add_library(driver STATIC
    src/device_driver.cpp
    src/spi_emulator.cpp
    src/adc_emulator.cpp
)

target_include_directories(driver PUBLIC ${PROJECT_SOURCE_DIR}/driver/include)
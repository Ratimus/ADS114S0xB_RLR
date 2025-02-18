# tests/CMakeLists.txt
enable_testing()

# Add GoogleTest (using FetchContent)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

add_executable(test_driver
    test_driver.cpp
)

# Link the test executable to GoogleTest and the driver static library
target_link_libraries(test_driver
    PRIVATE
    driver
    gtest
    gtest_main
)

# Register the test with CTest (so it can be run using `ctest`)
add_test(NAME TestDeviceDriver COMMAND test_driver)


# Create the executable for SPI tests
add_executable(test_spi
    test_spi.cpp
)

# Link the SPI test executable to GoogleTest and the driver static library
target_link_libraries(test_spi
    PRIVATE
    driver
    gtest
    gtest_main
)

# Register the SPI test with CTest
add_test(NAME TestSPI COMMAND test_spi)

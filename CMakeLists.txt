# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(ADS114S0xB_RLR VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(driver)
add_subdirectory(app)
add_subdirectory(tests)

enable_testing()
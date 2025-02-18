# app/CMakeLists.txt
add_executable(app
    src/main.cpp
)

target_link_libraries(app PRIVATE driver)

target_include_directories(app PRIVATE ${PROJECT_SOURCE_DIR}/driver/include)

cmake_minimum_required(VERSION 3.10)
project(DistillerGUI)

# Add configuration paths
add_definitions(-DLV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")

set(LVGL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lvgl)

# Add LVGL library
file(GLOB_RECURSE LVGL_SOURCES 
    ${LVGL_DIR}/src/*.c
)
add_library(lvgl STATIC ${LVGL_SOURCES})

# Include directories for LVGL
target_include_directories(lvgl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LVGL_DIR}
    ${LVGL_DIR}/src
    ${LVGL_DIR}/src/core
    ${LVGL_DIR}/src/draw
    ${LVGL_DIR}/src/misc
    ${LVGL_DIR}/src/hal
    ${LVGL_DIR}/src/font
    ${LVGL_DIR}/src/widgets
    ${LVGL_DIR}/src/extra
    ${LVGL_DIR}/src/display
)

# Create executable
add_executable(DistillerGUI 
    main.c
    monorama_14.c
    eink_driver.c
    lodepng.c
    background.c
)

# Include directories for the main executable
target_include_directories(DistillerGUI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LVGL_DIR}
    ${LVGL_DIR}/src
    ${LVGL_DIR}/src/core
    ${LVGL_DIR}/src/hal
    ${LVGL_DIR}/src/misc
    ${LVGL_DIR}/src/draw
    ${LVGL_DIR}/src/font
    ${LVGL_DIR}/src/widgets
    ${LVGL_DIR}/src/display
)

# Link libraries
target_link_libraries(DistillerGUI PRIVATE lvgl pthread gpiod)

# Add debug flags
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g -O0)
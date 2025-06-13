cmake_minimum_required(VERSION 3.10)
project(DistillerGUI)

# ALSA and CURL packages removed - audio and api_client modules deleted

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

# Create executable first
add_executable(DistillerGUI 
    main.c
    eink_driver.c
    lodepng.c
    background.c
    font_distiller.c
    system_monitor.c
    dashboard_ui.c
)

# Then add Montserrat font configuration
target_compile_definitions(${PROJECT_NAME} PRIVATE
    # Remove the font definitions since they're in lv_conf.h
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

# Detect libgpiod version
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)
string(REGEX MATCH "^([0-9]+)" GPIOD_MAJOR_VERSION "${GPIOD_VERSION}")
if(GPIOD_MAJOR_VERSION GREATER_EQUAL 2)
    message(STATUS "Using libgpiod v2.x API")
    target_compile_definitions(DistillerGUI PRIVATE GPIOD_V2)
else()
    message(STATUS "Using libgpiod v1.x API")
endif()

# Set LOCAL_DEBUG flag
# target_compile_definitions(DistillerGUI PRIVATE LOCAL_DEBUG)

# Link libraries
target_link_libraries(DistillerGUI PRIVATE 
    lvgl 
    pthread 
    gpiod 
)

# Add debug flags
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g -O0)

cmake_minimum_required(VERSION 3.10)
project(inih VERSION 60.0.0 LANGUAGES C)

add_library(${PROJECT_NAME} STATIC
    ini.c
)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
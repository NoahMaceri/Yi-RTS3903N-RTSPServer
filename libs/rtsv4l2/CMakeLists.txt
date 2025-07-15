cmake_minimum_required(VERSION 3.10)

project(rtsv4l2)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME}  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)
target_link_libraries(${PROJECT_NAME} INTERFACE
    librtsv4l2.so
)
cmake_minimum_required(VERSION 3.10)

project(extern_libs)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/h1_encoder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/aacenc)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/asound)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/opencore-amrnb)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/opus)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtsaec)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtsbmp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtscamkit)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtscrypt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtsgeom)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtsio)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtsmp3)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtsnm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtsstream)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtsutils)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtsv4l2)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sbc)

# Create a target for the rstream library
add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE
    asound
    rtsio
    rtsstream
    rtscamkit
    rtsv4l2
    h1_encoder
    aacenc
    rtsaec
    rtsbmp
    rtscrypt
    rtsmp3
    opus
    sbc
    rtsgeom
    opencore-amrnb
#    rtsnm
    rtsutils
)
target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/h1_encoder/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libaacenc/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libasound/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libopencore-armnb/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libopus/include
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsaec/include
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsbmp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/librtscamkit/include
    ${CMAKE_CURRENT_SOURCE_DIR}/librtscrypt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsgeom/include
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsmp3/include
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsnm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsstream/include
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsutils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsv4l2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libsbc/include
)

# Create all libs
file(GLOB_RECURSE ${PROJECT_NAME}_ALL "${CMAKE_CURRENT_SOURCE_DIR}/*.so")
set(${PROJECT_NAME}_ALL "${${PROJECT_NAME}_ALL}" PARENT_SCOPE)
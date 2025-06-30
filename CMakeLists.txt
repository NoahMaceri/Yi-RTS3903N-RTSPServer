cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# PreLoad.cmake should have ran before this file, load the cross-compiler toolchain
include(third-party/rsdk/enable_mips_uclibc.cmake)

project(RTS3903N_RTSP)

# tell binarys to load .so from cwd
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'\$ORIGIN'")

# Get the Live555 library
add_subdirectory(third-party/live555)

# rRTSPServer
add_executable(rRTSPServer rRTSPServer/src/rRTSPServer.cpp)
target_link_libraries(rRTSPServer
    groupsock
    BasicUsageEnvironment
    liveMedia
    UsageEnvironment
)

# imager_streamer
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-rpath-link=${CMAKE_SOURCE_DIR}/lib/")
file(GLOB IMAGER_STREAMER_LIBS ${CMAKE_SOURCE_DIR}/imager_streamer/lib/*.so)
add_executable(imager_streamer
        ${CMAKE_SOURCE_DIR}/imager_streamer/src/stream.c
)
target_link_libraries(imager_streamer
        # -lrtsio -lopus -lrtsjpeg -lsbc -lasound -lrtsgeom -lrtstream -lrtsv4l2 -lrtscamkit -lh1encoder -lrtsosd -lrtsutils -lrtsosd2 -lrtsjpeg -lrtsisp -lrtsaec
        ${IMAGER_STREAMER_LIBS}
)
target_include_directories(imager_streamer PRIVATE
        ${CMAKE_SOURCE_DIR}/imager_streamer/include
        ${CMAKE_SOURCE_DIR}/imager_streamer/lib
)
target_link_directories(imager_streamer PRIVATE
        ${CMAKE_SOURCE_DIR}/imager_streamer/lib
)

add_custom_target(${PROJECT_NAME} ALL
    DEPENDS rRTSPServer imager_streamer
)

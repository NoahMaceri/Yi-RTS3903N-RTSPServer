cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# PreLoad.cmake should have ran before this file, load the cross-compiler toolchain
include(third-party/rsdk/enable_mips_uclibc.cmake)

project(RTS3903N_RTSP VERSION 0.3.0 LANGUAGES C CXX)

# handle ver.h.in
configure_file(
    ${CMAKE_SOURCE_DIR}/ver.h.in
    ${CMAKE_BINARY_DIR}/ver.h
)

# -- THIRD PARTY LIBRARIES --
# Live555
add_subdirectory(third-party/live555)
# inih
add_subdirectory(third-party/inih)
# zlog
add_subdirectory(third-party/zlog)
include_directories(${CMAKE_SOURCE_DIR}/third-party/zlog/src)
# extern_libs
add_subdirectory(libs)

# -- EXECUTABLES --
include_directories(
        ${CMAKE_BINARY_DIR}
        src/includes
        src/rtsp_server/include
)
# rtsp_server
# glob for all cpp
file(GLOB_RECURSE rtsp_server_srcs ${CMAKE_SOURCE_DIR}/src/rtsp_server/src/*.cpp)
add_executable(rtsp_server
    ${rtsp_server_srcs}
)
target_link_libraries(rtsp_server
    groupsock
    BasicUsageEnvironment
    liveMedia
    UsageEnvironment
    inih
    zlog
    extern_libs
)

# audio_streamer
add_executable(audio_streamer
        src/audio_streamer/src/audio_streamer.c
)
target_link_libraries(audio_streamer
        inih
        zlog
        extern_libs
)

# imager_streamer
add_executable(imager_streamer
        src/imager_streamer/src/imager_streamer.c
)
target_link_libraries(imager_streamer
        inih
        zlog
        extern_libs
)

# -- PACKAGING --
add_custom_target(package_${PROJECT_NAME} COMMAND
        # if out folder exists, remove it
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/out
        # create out folder
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/out
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/out/lib
        # -- rRTSPServer --
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/rtsp_server
            ${CMAKE_BINARY_DIR}/out
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/third-party/live555/*.so
            ${CMAKE_BINARY_DIR}/out/lib
        # -- image_streamer --
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/imager_streamer
            ${CMAKE_BINARY_DIR}/out
        COMMAND ${CMAKE_COMMAND} -E copy
            ${extern_libs_ALL}
            ${CMAKE_BINARY_DIR}/out/lib
        COMMAND ${CMAKE_COMMAND} -E copy
            ${TOOLCHAIN_FOLDER}/lib/libstdc++.so.6
            ${CMAKE_BINARY_DIR}/out/lib
        COMMAND ${CMAKE_COMMAND} -E copy
            ${TOOLCHAIN_FOLDER}/lib/libatomic.so.1
            ${CMAKE_BINARY_DIR}/out/lib
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/third-party/zlog/lib/libzlog.so.1.2.*
            ${CMAKE_BINARY_DIR}/out/lib/libzlog.so.1
        # We have to rename some libraries to match the expected names
        COMMAND ${CMAKE_COMMAND} -E rename
            ${CMAKE_BINARY_DIR}/out/lib/librtsosd.so
            ${CMAKE_BINARY_DIR}/out/lib/librtsosd.so.1
        COMMAND ${CMAKE_COMMAND} -E rename
            ${CMAKE_BINARY_DIR}/out/lib/librtsio.so
            ${CMAKE_BINARY_DIR}/out/lib/librtsio.so.0
        COMMAND ${CMAKE_COMMAND} -E rename
            ${CMAKE_BINARY_DIR}/out/lib/libopus.so
            ${CMAKE_BINARY_DIR}/out/lib/libopus.so.0
        COMMAND ${CMAKE_COMMAND} -E rename
            ${CMAKE_BINARY_DIR}/out/lib/libsbc.so
            ${CMAKE_BINARY_DIR}/out/lib/libsbc.so.1
        COMMAND ${CMAKE_COMMAND} -E rename
            ${CMAKE_BINARY_DIR}/out/lib/librtsisp.so
            ${CMAKE_BINARY_DIR}/out/lib/librtsisp.so.1
        COMMAND ${CMAKE_COMMAND} -E rename
            ${CMAKE_BINARY_DIR}/out/lib/librtscrypt.so
            ${CMAKE_BINARY_DIR}/out/lib/librtscrypt.so.1
        COMMAND ${CMAKE_COMMAND} -E rename
            ${CMAKE_BINARY_DIR}/out/lib/libopencore-amrnb.so
            ${CMAKE_BINARY_DIR}/out/lib/libopencore-amrnb.so.0
        # -- sd_card payload --
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/sd_payload/
            ${CMAKE_BINARY_DIR}/out

        # Create a tar
        COMMAND tar -cf ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}.tar -C ${CMAKE_BINARY_DIR}/out .

        # Clean up
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/out

        # Print a message indicating the package is ready
        COMMAND ${CMAKE_COMMAND} -E echo "Package created at ${CMAKE_BINARY_DIR}/out/${PROJECT_NAME}-${PROJECT_VERSION}.tar"
        DEPENDS rtsp_server imager_streamer
)

add_custom_target(${PROJECT_NAME} COMMAND
    DEPENDS rtsp_server imager_streamer package_${PROJECT_NAME}
)

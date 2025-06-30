cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(live)

# First download the zip
set(SOURCE_VER "2021.04.06")

# if the zip is not present, download it
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/live.${SOURCE_VER}.tar.gz")
    message(STATUS "Downloading live source version ${SOURCE_VER}")
    file(DOWNLOAD
            "https://download.videolan.org/pub/contrib/live555/live.${SOURCE_VER}.tar.gz"
            "${CMAKE_CURRENT_BINARY_DIR}/live.${SOURCE_VER}.tar.gz"
    )
    message(STATUS "Download complete")
endif()

# Extract the source code
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/live")
    message(STATUS "Extracting live source")
    file(ARCHIVE_EXTRACT
            INPUT "${CMAKE_CURRENT_BINARY_DIR}/live.${SOURCE_VER}.tar.gz"
            DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    message(STATUS "Extraction complete")
endif()

add_library(compiler_flags INTERFACE)
target_compile_options(compiler_flags INTERFACE
        -O1
)
target_compile_definitions(compiler_flags INTERFACE
    -DNO_STD_LIB
    -DBSD=1
    -DUSE_SYSTEM_RANDOM
    -DSOCKLEN_T=socklen_t
    -D_LARGEFILE_SOURCE=1
    -D_FILE_OFFSET_BITS=64
    -DNO_OPENSSL=1
    -DNEWLOCALE_NOT_USED
)

set(all_include_dirs
    "${CMAKE_CURRENT_SOURCE_DIR}/live/UsageEnvironment/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/live/BasicUsageEnvironment/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/live/groupsock/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/live/liveMedia/include"
)

file(GLOB BASIC_USAGE_ENVIRONMENT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/live/BasicUsageEnvironment/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/live/BasicUsageEnvironment/*.c)
add_library(BasicUsageEnvironment SHARED ${BASIC_USAGE_ENVIRONMENT_SRCS})
target_include_directories(BasicUsageEnvironment PUBLIC ${all_include_dirs})
target_link_libraries(BasicUsageEnvironment PUBLIC compiler_flags)

file(GLOB USAGE_ENVIRONMENT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/live/UsageEnvironment/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/live/UsageEnvironment/*.c)
add_library(UsageEnvironment SHARED ${USAGE_ENVIRONMENT_SRCS})
target_include_directories(UsageEnvironment PUBLIC ${all_include_dirs})
target_link_libraries(UsageEnvironment PUBLIC compiler_flags)

file(GLOB GROUPSOCK_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/live/groupsock/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/live/groupsock/*.c)
add_library(groupsock SHARED ${GROUPSOCK_SRCS})
target_include_directories(groupsock PUBLIC ${all_include_dirs})
target_link_libraries(groupsock PUBLIC compiler_flags)

file(GLOB LIVEMEDIA_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/live/liveMedia/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/live/liveMedia/*.c)
add_library(liveMedia SHARED ${LIVEMEDIA_SRCS})
target_include_directories(liveMedia PUBLIC ${all_include_dirs})
target_link_libraries(liveMedia PUBLIC compiler_flags)

# Copy the .so files to the build directory on build
add_custom_command(TARGET BasicUsageEnvironment POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libBasicUsageEnvironment.so ${CMAKE_BINARY_DIR}
)
add_custom_command(TARGET UsageEnvironment POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libUsageEnvironment.so ${CMAKE_BINARY_DIR}
)
add_custom_command(TARGET groupsock POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libgroupsock.so ${CMAKE_BINARY_DIR}
)
add_custom_command(TARGET liveMedia POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libUsageEnvironment.so ${CMAKE_BINARY_DIR}
)


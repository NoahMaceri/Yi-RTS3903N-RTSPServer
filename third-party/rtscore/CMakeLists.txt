cmake_minimum_required(VERSION 3.10)

project(rtscore)

# h1_encoder
add_library(h1_encoder INTERFACE)
target_include_directories(h1_encoder INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/h1_encoder/include
)
target_link_directories(h1_encoder INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/h1_encoder/lib
)
target_link_libraries(h1_encoder INTERFACE
    libh1encoder.so
)

# libaacenc
add_library(libaacenc INTERFACE)
target_include_directories(libaacenc INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/libaacenc/include
)
target_link_directories(libaacenc INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/libaacenc/lib
)
target_link_libraries(libaacenc INTERFACE
    libaacenc.so
)

# librtsaec
add_library(librtsaec INTERFACE)
target_include_directories(librtsaec INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsaec/include
)
target_link_directories(librtsaec INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsaec/lib
)
target_link_libraries(librtsaec INTERFACE
    librtsaec.so
)

# librtsbmp
add_library(librtsbmp INTERFACE)
target_include_directories(librtsbmp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsbmp/include
)
target_link_directories(librtsbmp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsbmp/lib
)
target_link_libraries(librtsbmp INTERFACE
    librtsbmp.so
)

# librtscamkit
add_library(librtscam INTERFACE)
target_include_directories(librtscam INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtscamkit/include
)
target_link_directories(librtscam INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtscamkit/lib
)
target_link_libraries(librtscam INTERFACE
    librtscamkit.so
)

# librtscrypt
add_library(librtscrypt INTERFACE)
target_include_directories(librtscrypt INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtscrypt/include
)
target_link_directories(librtscrypt INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtscrypt/lib
)
target_link_libraries(librtscrypt INTERFACE
    librtscrypt.so
)

# librtsio
add_library(librtsio INTERFACE)
target_include_directories(librtsio INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsio/include
)
target_link_directories(librtsio INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsio/lib
)
target_link_libraries(librtsio INTERFACE
    librtsio.so
)

# librtsisp
add_library(librtsisp INTERFACE)
target_include_directories(librtsisp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsisp/include
)
target_link_directories(librtsisp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsisp/lib
)
target_link_libraries(librtsisp INTERFACE
    librtsisp.so
    librtsjpeg.so
    librtsosd2.so
    librtsosd.so
)

# librtsmp3
add_library(librtsmp3 INTERFACE)
target_include_directories(librtsmp3 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsmp3/include
)
target_link_directories(librtsmp3 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsmp3/lib
)
target_link_libraries(librtsmp3 INTERFACE
    librtsmp3.so
)

# librtstream
add_library(librtstream INTERFACE)
target_include_directories(librtstream INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtstream/include
)
target_link_directories(librtstream INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtstream/lib
)
target_link_libraries(librtstream INTERFACE
    librtstream.so
)

# librtsv4l2
add_library(librtsv4l2 INTERFACE)
target_include_directories(librtsv4l2 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsv4l2/include
)
target_link_directories(librtsv4l2 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/librtsv4l2/lib
)
target_link_libraries(librtsv4l2 INTERFACE
    librtsv4l2.so
)

# other_libs
add_library(other_libs INTERFACE)
target_link_directories(other_libs INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/other_libs/lib
)
target_link_libraries(other_libs INTERFACE
    libopus.so
    libsbc.so
    libasound.so
    librtsgeom.so
    libopencore-amrnb.so
# For some reason, this specific library throws an error on linking but IS needed at runtime
#    librtsnm.so
    librtsutils.so

)

# Create a target for the rstream library
add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE
    librtsio
    librtstream
    librtscam
    librtsv4l2
    h1_encoder
    librtsisp
    libaacenc
    librtsaec
#    librtsbmp
#    librtscrypt
    librtsmp3
    other_libs
)

# Create all libs
set(${PROJECT_NAME}_LIBS "" PARENT_SCOPE)
file(GLOB_RECURSE ${PROJECT_NAME}_LIBS "${CMAKE_CURRENT_SOURCE_DIR}/*.so")
set(${PROJECT_NAME}_LIBS "${${PROJECT_NAME}_LIBS}" PARENT_SCOPE)
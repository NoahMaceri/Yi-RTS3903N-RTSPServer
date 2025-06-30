# Set up the toolchain for building 32bit MIPS-I
# This file will set up the toolchain for building 32bit MIPS-I
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CROSSCOMPILING TRUE)
# check if TOOLCHAIN_VER is set, if not set it to a default value
if(NOT DEFINED TOOLCHAIN_VER)
    message(STATUS "TOOLCHAIN_VER is not defined, check the PreLoad.cmake file")
endif()
if(NOT DEFINED TOOLCHAIN_FOLDER)
    message(STATUS "TOOLCHAIN_VER is not defined, check the PreLoad.cmake file")
endif()
set(CMAKE_C_COMPILER    ${TOOLCHAIN_FOLDER}/bin/mips-linux-uclibc-gcc)
set(CMAKE_CXX_COMPILER  ${TOOLCHAIN_FOLDER}/bin/mips-linux-uclibc-g++)
set(CMAKE_LINKER        ${TOOLCHAIN_FOLDER}/bin/mips-linux-uclibc-ld)
set(CMAKE_AR            ${TOOLCHAIN_FOLDER}/bin/mips-linux-uclibc-ar)
set(CMAKE_ASM_COMPILER  ${TOOLCHAIN_FOLDER}/bin/mips-linux-uclibc-as)
set(CMAKE_STRIP         ${TOOLCHAIN_FOLDER}/bin/mips-linux-uclibc-strip)
set(CMAKE_SYSROOT_DIR   ${TOOLCHAIN_FOLDER}/mips-linux-uclibc)

set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT_DIR})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set the target architecture to MIPS-I for 5281
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=5281" CACHE STRING "C compiler flags for MIPS architecture")
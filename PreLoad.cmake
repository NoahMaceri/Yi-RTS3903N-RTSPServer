cmake_minimum_required(VERSION 3.10)

# Extract the RSDK toolchain if not already done
set(TOOLCHAIN_VER "rsdk-4.8.5-5281-EL-3.10-u0.9.33-m32fut-161202" CACHE STRING "RSDK toolchain version")

# Extract the source code
set(TOOLCHAIN_FOLDER "${CMAKE_SOURCE_DIR}/third-party/rsdk/${TOOLCHAIN_VER}" CACHE PATH "RSDK toolchain folder")
if(NOT EXISTS "${TOOLCHAIN_FOLDER}")
    message(STATUS "Extracting rsdk toolchain version ${TOOLCHAIN_VER}")
    file(ARCHIVE_EXTRACT
            INPUT "${TOOLCHAIN_FOLDER}.tar.gz"
            DESTINATION "${CMAKE_SOURCE_DIR}/third-party/rsdk/"
    )
    message(STATUS "rsdk toolchain extraction complete")
endif()
message(STATUS "RSDK toolchain version ${TOOLCHAIN_VER}")
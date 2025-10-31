cmake_minimum_required(VERSION 3.15)
project(MultiToonLib C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Print info
message(STATUS "Source dir: ${CMAKE_SOURCE_DIR}")

# Detect platform
if(WIN32)
        message(STATUS "Detected: native Windows build")
        set(PLATFORM "windows")
elseif(UNIX AND NOT MINGW)
        message(STATUS "Detected: Linux build")
        set(PLATFORM "linux")
elseif(MINGW)
        message(STATUS "Detected: cross-compile for Windows")
        set(PLATFORM "windows")
else()
        message(FATAL_ERROR "Unknown platform")
endif()

# Select sources
if(${PLATFORM} STREQUAL "linux")
        set(SOURCES
                ${CMAKE_SOURCE_DIR}/src/mtlib.c
                ${CMAKE_SOURCE_DIR}/src/mtlib_linux.c
        )
elseif(${PLATFORM} STREQUAL "windows")
        set(SOURCES
                ${CMAKE_SOURCE_DIR}/src/mtlib.c
                ${CMAKE_SOURCE_DIR}/src/mtlib_windows.c
        )
endif()

# Print found sources
message(STATUS "Using sources: ${SOURCES}")

# Validate
foreach(f ${SOURCES})
        if(NOT EXISTS ${f})
                message(FATAL_ERROR "Missing source file: ${f}")
        endif()
endforeach()

# Add library
add_library(mtlib SHARED ${SOURCES})
set_target_properties(mtlib PROPERTIES OUTPUT_NAME multitoon)

# Link libraries
if(${PLATFORM} STREQUAL "linux")
        target_link_libraries(mtlib PRIVATE X11 xdo)
elseif(${PLATFORM} STREQUAL "windows")
        target_link_libraries(mtlib PRIVATE user32 kernel32)
endif()

# linux install
if(UNIX AND NOT MINGW)
    install(TARGETS mtlib
        LIBRARY DESTINATION /usr/lib
        ARCHIVE DESTINATION /usr/lib
        RUNTIME DESTINATION /usr/bin
    )

    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION /usr/include/multitoon
        FILES_MATCHING PATTERN "*.h"
    )
endif()


cmake_minimum_required(VERSION 3.15)
project(MultiToonLib C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include_directories(${CMAKE_SOURCE_DIR}/include)

message(STATUS "Source dir: ${CMAKE_SOURCE_DIR}")

if(WIN32)
        message(STATUS "Detected: native Windows build")
        set(PLATFORM "windows")

elseif(UNIX AND NOT MINGW AND NOT APPLE)
        message(STATUS "Detected: Linux build")
        set(PLATFORM "linux")

elseif(APPLE)
        message(STATUS "Detected: macOS build")
        set(PLATFORM "macos")

elseif(MINGW)
        message(STATUS "Detected: cross-compile for Windows")
        set(PLATFORM "windows")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        message(STATUS "Detected: cross-compile for macOS via osxcross")
        set(PLATFORM "macos")

else()
        message(FATAL_ERROR "Unknown platform")
endif()

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
elseif(${PLATFORM} STREQUAL "macos")
        set(SOURCES
                ${CMAKE_SOURCE_DIR}/src/mtlib.c
                ${CMAKE_SOURCE_DIR}/src/mtlib_macos.c
        )
endif()

message(STATUS "Using sources: ${SOURCES}")

foreach(f ${SOURCES})
        if(NOT EXISTS ${f})
                message(FATAL_ERROR "Missing source file: ${f}")
        endif()
endforeach()

add_library(mtlib SHARED ${SOURCES})
set_target_properties(mtlib PROPERTIES OUTPUT_NAME multitoon)

if(${PLATFORM} STREQUAL "linux")
        target_link_libraries(mtlib PRIVATE X11 xdo)
elseif(${PLATFORM} STREQUAL "windows")
        target_link_libraries(mtlib PRIVATE user32 kernel32)

elseif(${PLATFORM} STREQUAL "macos")
        find_library(CARBON_FRAMEWORK Carbon)
        find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
        find_library(COREGRAPHICS_FRAMEWORK CoreGraphics)

if(NOT CARBON_FRAMEWORK OR NOT COREFOUNDATION_FRAMEWORK OR NOT COREGRAPHICS_FRAMEWORK)
        message(FATAL_ERROR "Missing one or more required macOS frameworks (Carbon, CoreFoundation, CoreGraphics)")
endif()

        target_link_libraries(mtlib PRIVATE
        ${CARBON_FRAMEWORK}
        ${COREFOUNDATION_FRAMEWORK}
        ${COREGRAPHICS_FRAMEWORK}
)
endif()

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

add_executable(mt_test ${CMAKE_SOURCE_DIR}/src/main.c)
target_link_libraries(mt_test PRIVATE mtlib xdo X11)


cmake_minimum_required(VERSION 3.10)
project(MultiToonLib C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Include Dirs
include_directories(${PROJECT_SOURCE_DIR}/include)

# Build shared libs
add_library(MultiToonLib SHARED
        src/multitoonlib.c
        src/mtlib_linux.c
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(XDO REQUIRED libxdo)
target_include_directories(MultiToonLib PRIVATE ${XDO_INCLUDE_DIRS})
target_link_libraries(MultiToonLib PRIVATE ${XDO_LIBRARIES})

install(TARGETS MultiToonLib
        LIBRARY DESTINATION /usr/lib
        ARCHIVE DESTINATION /usr/lib)

install(DIRECTORY include/
        DESTINATION /usr/include
        FILES_MATCHING PATTERN "*.h")

install(CODE "execute_process(COMMAND ldconfig)")


add_executable(xdo_test src/main.c)

# Include dirs for your own headers and libxdo
target_include_directories(xdo_test PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${XDO_INCLUDE_DIRS}
)

# Link against both your library and libxdo
target_link_libraries(xdo_test PRIVATE
    MultiToonLib
    ${XDO_LIBRARIES}
)

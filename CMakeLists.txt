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

add_executable(testmain src/main.c)
target_include_directories(testmain PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(testmain PRIVATE MultiToonLib ${XDO_LIBRARIES})

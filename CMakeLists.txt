cmake_minimum_required(VERSION 3.16)
project(chip8)

# Setup the compiler
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-Wno-format -Wall)

# Search source files and store them in the SOURCES variable
file(GLOB_RECURSE SOURCES "src/*.cpp")

# This is our target for the executable file
add_executable(chip8 ${SOURCES})

# Add the SDL2 cmake files to the cmake path, so the SDL2 files can be found and added
list(APPEND CMAKE_PREFIX_PATH "C:\\vclib\\SDL2-2.30.7\\cmake")
find_package(SDL2 REQUIRED)

target_include_directories(chip8 PUBLIC include)
target_include_directories(chip8 PUBLIC ${SDL2_INCLUDE_DIRS})

target_link_libraries(chip8 PUBLIC ${SDL2_LIBRARIES})

cmake_minimum_required(VERSION 3.13)

project(bittorrent-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

# Include FetchContent module
include(FetchContent)

# Fetch Range-v3 library
FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_TAG        0.12.0 # Specify the desired version tag or commit hash
)

# Make Range-v3 available
FetchContent_MakeAvailable(range-v3)

add_executable(bittorrent ${SOURCE_FILES})

# Link Range-v3 library
target_link_libraries(bittorrent PRIVATE range-v3)
cmake_minimum_required(VERSION 3.13)

project(bittorrent-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

# Include FetchContent module
include(FetchContent)

# Fetch Range-v3 library
set(RANGES_ENABLE_TESTS OFF)  # Disable tests for range-v3
FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_TAG        0.12.0 
)

# Fetch httplib library
set(CPR_ENABLE_TESTS OFF)  # Disable tests for cpr
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.15.3
)


# Fetch cpr library
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG        1.10.0 
)

# Make Range-v3, curl, and cpr available
FetchContent_MakeAvailable(range-v3 httplib cpr)

add_executable(bittorrent ${SOURCE_FILES})

# Link Range-v3, curl, and cpr libraries
target_link_libraries(bittorrent PRIVATE range-v3 httplib cpr::cpr)

target_include_directories(bittorrent PRIVATE  ${CMAKE_SOURCE_DIR}/src)

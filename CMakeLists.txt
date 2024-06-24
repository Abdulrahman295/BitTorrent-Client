cmake_minimum_required(VERSION 3.13)

project(bittorrent-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
# set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_MESSAGE_LOG_LEVEL NOTICE)
# set(CMAKE_VERBOSE_MAKEFILE OFF)
# set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

# Include FetchContent module
include(FetchContent)

# Fetch Range-v3 library
FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_TAG        0.12.0 
)

# Fetch httplib library
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

# Disable tests and reduce output
set(RANGE_V3_TESTS OFF CACHE BOOL "Enable tests for Range-v3" FORCE)
set(RANGE_V3_EXAMPLES OFF CACHE BOOL "Enable examples for Range-v3" FORCE)
set(RANGE_V3_HEADER_ONLY ON CACHE BOOL "Use range-v3 as header-only" FORCE)

set(CPR_BUILD_TESTS OFF CACHE BOOL "Set to ON to build cpr tests" FORCE)

set(CURL_DISABLE_TESTS ON CACHE BOOL "Disable curl tests" FORCE)
set(CURL_VERBOSE_LOGGING OFF CACHE BOOL "Disable verbose logging" FORCE)
set(CURL_DISABLE_VERBOSE_STRINGS ON CACHE BOOL "Disable verbose strings" FORCE)

# Make Range-v3, curl, and cpr available
FetchContent_MakeAvailable(range-v3 httplib cpr)

add_executable(bittorrent ${SOURCE_FILES})

# Link Range-v3, curl, and cpr libraries
target_link_libraries(bittorrent PRIVATE range-v3 httplib cpr::cpr)

target_include_directories(bittorrent PRIVATE  ${CMAKE_SOURCE_DIR}/src)

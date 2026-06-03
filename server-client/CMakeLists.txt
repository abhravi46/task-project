cmake_minimum_required(VERSION 3.10)
project(ServerClient)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files from app directory recursively
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/app/*.cpp"
)

# Collect all header files for IDE support
file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/app/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/*.h"
)

# Create the executable
add_executable(server_client ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(server_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/app/framework
    ${CMAKE_CURRENT_SOURCE_DIR}/app/interface
)

# Set output directory
set_target_properties(server_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Optional: Enable warnings
if(MSVC)
    target_compile_options(server_client PRIVATE /W4)
else()
    target_compile_options(server_client PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Print summary
message(STATUS "Source files found:")
foreach(source ${SOURCES})
    message(STATUS "  ${source}")
endforeach()

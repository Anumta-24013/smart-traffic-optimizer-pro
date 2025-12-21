cmake_minimum_required(VERSION 3.10)

project(SmartTrafficOptimizer VERSION 1.0.0 LANGUAGES CXX)

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Main executable
add_executable(traffic_optimizer main.cpp)

# Platform-specific linking
if(WIN32)
    target_link_libraries(traffic_optimizer ws2_32)
else()
    target_link_libraries(traffic_optimizer pthread)
endif()

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Install
install(TARGETS traffic_optimizer DESTINATION bin)
install(DIRECTORY data/ DESTINATION data)
install(DIRECTORY frontend/ DESTINATION frontend)

# CPack for packaging
set(CPACK_PACKAGE_NAME "SmartTrafficOptimizer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION "Smart Traffic Route Optimizer for Pakistan Cities")
include(CPack)

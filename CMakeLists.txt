cmake_minimum_required(VERSION 3.1.0)

project(Daikin-ir-control-rpi)

# Include all extra cmake file.
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find pigpio
if (CMAKE_SYSTEM_NAME STREQUAL RPI)
	find_package(pigpio QUIET)
endif ()

# Find boost.
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS program_options)

# Add sub directory.
add_subdirectory(app)
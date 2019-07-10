cmake_minimum_required(VERSION 3.1.0)

project(Daikin-ir-control-rpi)

# Include all extra cmake file.
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find pigpio
if (UNIX)
	find_package(pigpio QUIET)

	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads QUIET)
endif ()

# Find boost.
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS program_options)

# Add sub directory.
add_subdirectory(app)

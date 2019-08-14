set(TARGET_NAME arc433A59_ir_tx)

# Populate a CMake variable with the sources
set(SRCS arc433A59_ir_tx.cpp ITxSender_testfile.cpp ITxSender.cpp SignalFactory.cpp)
set(HEADERS ITxSender.h ITxSender_testfile.h SignalFactory.h)

# boost.
if (Boost_PROGRAM_OPTIONS_FOUND)
	set(INCS ${Boost_INCLUDE_DIRS})
	set(LIBS ${Boost_PROGRAM_OPTIONS_LIBRARIES})
else ()
	message(WARNING "Cannot find boost::program_options on system. The ${TARGET_NAME} is disabled.")
	return()
endif ()

# Find pigpio
if (pigpio_FOUND AND THREADS_FOUND)
	set (SRCS ${SRCS} ITxSender_pigpiod.cpp)
	set (HEADERS ${HEADERS} ITxSender_pigpio.h)
	set(INCS ${INCS} ${pigpio_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${pigpiod_if2_LIBRARY} Threads::Threads)
	add_definitions(-DHAVE_PIGPIO)
else ()
	if (CMAKE_SYSTEM_NAME STREQUAL Windows OR CMAKE_SYSTEM_NAME STREQUAL Darwin)
		message(WARNING "The target system is not Raspberry PI. The GPIO output is not support.")
	elseif (NOT pigpio_FOUND)
		message(WARNING "Cannot find pigpio on system. The GPIO output is not support.")
	elseif (NOT THREADS_FOUND)
		message(WARNING "Cannot find pthread on system. The GPIO output is not support.")
	endif ()
endif ()

if (MSVC OR XCODE)
	set(SRCS ${SRCS} ${HEADERS})
endif ()

include_directories(${INCS})
add_executable(${TARGET_NAME} ${SRCS})
target_link_libraries(${TARGET_NAME} ${LIBS})

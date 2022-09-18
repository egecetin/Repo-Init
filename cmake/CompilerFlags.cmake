include(LinkTimeOptimization)

link_time_optimization(OPTIONAL)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

# Intel Compiler
if(CMAKE_C_COMPILER_ID STREQUAL "Intel")
	message("${BoldBlue}Intel C Compiler detected. Updating flags ...${ColourReset}")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -xHost -use-intel-optimized-headers -static-intel")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O0 -g -xHost -ipo -use-intel-optimized-headers -static-intel")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	message("${BoldBlue}Intel C++ Compiler detected. Updating flags ...${ColourReset}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3 -xHost -use-intel-optimized-headers -static-intel")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -xHost -ipo -use-intel-optimized-headers -static-intel")
endif()

# GNU
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	message("${BoldBlue}GNU C Compiler detected. Updating flags ...${ColourReset}")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g --coverage -fdiagnostics-color=always")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -fdiagnostics-color=always")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	message("${BoldBlue}GNU C++ Compiler detected. Updating flags ...${ColourReset}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g --coverage -fdiagnostics-color=always")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fdiagnostics-color=always")
endif()

# MSVC

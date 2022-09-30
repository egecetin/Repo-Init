include(LinkTimeOptimization)

link_time_optimization()
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

# GNU
if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
	message("${BoldBlue}GNU C Compiler detected. Updating flags ...${ColourReset}")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g --coverage -fdiagnostics-color=always")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -fdiagnostics-color=always")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
	message("${BoldBlue}GNU C++ Compiler detected. Updating flags ...${ColourReset}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g --coverage -fdiagnostics-color=always")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fdiagnostics-color=always")
endif()

# Intel Compiler
if(CMAKE_C_COMPILER_ID MATCHES "Intel")
	message("${BoldBlue}Intel C Compiler detected. Updating flags ...${ColourReset}")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -xHost")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -xHost")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
	message("${BoldBlue}Intel C++ Compiler detected. Updating flags ...${ColourReset}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -xHost")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -xHost")
endif()

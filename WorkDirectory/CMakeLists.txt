project(XXX)
cmake_minimum_required(VERSION 3.16)

# CMake Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(Colorize)
include(GitVersion)
include(SvnVersion)
include(AutoBuildNumber)

if(NOT DEFINED REVISION_VERSION)
    message(SEND_ERROR "Can't find SVN or Git. The folder is not under version control.")
endif()

# Set properties
set(CMAKE_PROJECT_VERSION "1.0.${BUILD_VERSION}.rev-${REVISION_VERSION}")
message("${BoldBlue}Compiling Version ${CMAKE_PROJECT_VERSION}${ColourReset}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/bin)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Subdirectories
# add_subdirectory(thirdparty/)

# Update build number at every build (Should be at the end of file!)
add_custom_command(
    TARGET XXX
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -P "cmake/AutoBuildNumber.cmake"
)

cmake_minimum_required(VERSION 3.16)
project(XXX)

# CMake Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(Colorize)
include(GitVersion)
include(SvnVersion)

if(NOT DEFINED REVISION_VERSION)
    message(SEND_ERROR "Can't find SVN or Git. The folder is not under version control.")
endif()

# Set properties
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
if(EXISTS "../BuildNumberCache.txt")
    file(READ "BuildNumberCache.txt" BUILD_NUMBER)
else()
    set(BUILD_NUMBER "1")
endif()
set(PROJECT_VERSION_PATCH ${BUILD_NUMBER})
set(PROJECT_VERSION_TWEAK ${REVISION_VERSION})
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${BUILD_NUMBER}-rev${REVISION_VERSION}")
message("${BoldBlue}Compiling Version ${PROJECT_VERSION}${ColourReset}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib/static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib/shared)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib/static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib/shared)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/bin)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -DNDEBUG -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG -O3")

add_compile_options(-Wall -Wextra -pedantic)
configure_file(build_number.h.in build_number.h)

FILE(GLOB ProjectSources 
                ${PROJECT_SOURCE_DIR}/src/*.cpp 
                ${PROJECT_SOURCE_DIR}/src/*.c 
                ${PROJECT_SOURCE_DIR}/src/**/*.cpp 
                ${PROJECT_SOURCE_DIR}/src/**/*.c)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Subdirectories
# add_subdirectory(thirdparty/)

# Set Target
add_executable(XXX ${ProjectSources})
target_include_directories(XXX PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(XXX)

# Update build number at every build
add_custom_command(
    TARGET XXX
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -P "../cmake/AutoBuildNumber.cmake"
)

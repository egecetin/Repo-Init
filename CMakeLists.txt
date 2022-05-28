cmake_minimum_required(VERSION 3.16)
project(XXX)

# CMake Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(Colorize)
include(GitVersion)
include(SvnVersion)
include(Doxy)

if(NOT DEFINED REVISION_VERSION)
    message(SEND_ERROR "Can't find SVN or Git. The folder is not under version control.")
endif()

# Set project properties
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

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -DNDEBUG -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG -O3")

configure_file(build_number.h.in XXX_Version.h)

# Sources
FILE(GLOB ProjectSources 
                ${PROJECT_SOURCE_DIR}/src/*.cpp 
                ${PROJECT_SOURCE_DIR}/src/*.c 
                ${PROJECT_SOURCE_DIR}/src/**/*.cpp 
                ${PROJECT_SOURCE_DIR}/src/**/*.c)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(${PROJECT_SOURCE_DIR}/thirdparty/libzmq)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/cppzmq)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/rapidjson/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/spdlog/include)

# Disable/Enalbe options of subprojects
option(BUILD_TESTS "" OFF) # libzmq
option(WITH_DOCS "" OFF) # libzmq
option(CPPZMQ_BUILD_TESTS "" OFF)
option(RAPIDJSON_BUILD_DOC "" OFF)
option(RAPIDJSON_BUILD_EXAMPLES "" OFF)
option(RAPIDJSON_BUILD_TESTS "" OFF)
option(SPDLOG_BUILD_SHARED "" ON)

# Subdirectories
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/libzmq)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/cppzmq)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/rapidjson)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/spdlog)

# Add tests
add_subdirectory(${PROJECT_SOURCE_DIR}/tests)

# Set Target
add_library(XXX_Lib SHARED ${ProjectSources})
target_compile_options(XXX_Lib PUBLIC -Wall -Wextra -Werror -pedantic)
target_include_directories(XXX_Lib PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(XXX_Lib stdc++fs spdlog libzmq)

add_executable(XXX ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_compile_options(XXX PUBLIC -Wall -Wextra -Werror -pedantic)
target_include_directories(XXX PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(XXX XXX_Lib)

# Update build number at every build
add_custom_command(
    TARGET XXX_Lib
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -P "../cmake/AutoBuildNumber.cmake"
)

# Update dist folder after build
add_custom_command(
    TARGET XXX
    POST_BUILD
    COMMAND sh ../ldd-copy-dependencies.sh -b ../build/bin/Release/bin -t ../dist
)

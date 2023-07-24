cmake_minimum_required(VERSION 3.16)
project(XXX)

# CMake Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(Colorize)
include(CompilerOptions)
include(CompilerFlags)
include(CppStandards)
include(Doxy)
include(GitVersion)
include(GraphViz)
include(SvnVersion)

option(XXX_BUILD_TESTS "Build XXX tests" ON)
option(XXX_ENABLE_MEMLEAK_CHECK "Enables Memory Leak Checker for unit tests" OFF)

if(NOT DEFINED REVISION_VERSION)
  message(SEND_ERROR "Can't find SVN or Git. The folder is not under version control.")
endif()

# Set project properties
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)

if(EXISTS "${PROJECT_SOURCE_DIR}/BuildNumberCache.txt")
  file(READ "${PROJECT_SOURCE_DIR}/BuildNumberCache.txt" BUILD_NUMBER)
else()
  set(BUILD_NUMBER "1")
endif()

set(PROJECT_VERSION_PATCH ${BUILD_NUMBER})
set(PROJECT_VERSION_TWEAK ${REVISION_VERSION})
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${BUILD_NUMBER}-${REVISION_VERSION}")
message("${BoldBlue}Compiling Version ${PROJECT_VERSION}${ColourReset}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib/static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib/shared)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib/static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib/shared)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/bin)

# Set Compiler Properties
set(CMAKE_DEBUG_POSTFIX d)
cxx_14()

configure_file(build_number.h.in "Version.h")

# Define sources
file(GLOB ProjectSources
    ${PROJECT_SOURCE_DIR}/src/connection/Http.cpp
    ${PROJECT_SOURCE_DIR}/src/connection/RawSocket.cpp
    ${PROJECT_SOURCE_DIR}/src/connection/Zeromq.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/Logger.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/Loki.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/Sentry.cpp
    ${PROJECT_SOURCE_DIR}/src/metrics/Performance.cpp
    ${PROJECT_SOURCE_DIR}/src/metrics/PrometheusServer.cpp
    ${PROJECT_SOURCE_DIR}/src/metrics/Status.cpp
    ${PROJECT_SOURCE_DIR}/src/telnet/TelnetServer.cpp
    ${PROJECT_SOURCE_DIR}/src/telnet/TelnetStats.cpp
    ${PROJECT_SOURCE_DIR}/src/zeromq/ZeroMQServer.cpp
    ${PROJECT_SOURCE_DIR}/src/zeromq/ZeroMQStats.cpp
    ${PROJECT_SOURCE_DIR}/src/Control.cpp
    ${PROJECT_SOURCE_DIR}/src/Utils.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    )

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(${PROJECT_SOURCE_DIR}/thirdparty/cppzmq)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/date/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/http-status-codes)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/libzmq/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/prometheus-cpp/core/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/prometheus-cpp/pull/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/rapidjson/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/spdlog/include)
include_directories(${CMAKE_BINARY_DIR}/thirdparty/prometheus-cpp/core/include)
include_directories(${CMAKE_BINARY_DIR}/thirdparty/prometheus-cpp/pull/include)

# Disable/Enable options of subprojects
option(BUILD_SHARED_LIBS "" ON)
option(BUILD_STATIC "" OFF)
option(BUILD_TESTS "" OFF)

option(WITH_PERF_TOOL "" OFF)
option(WITH_DOCS "" OFF) # libzmq
option(RAPIDJSON_BUILD_DOC "" OFF)
option(RAPIDJSON_BUILD_EXAMPLES "" OFF)

option(CPPZMQ_BUILD_TESTS "" OFF)
option(RAPIDJSON_BUILD_TESTS "" OFF)
option(ENABLE_TESTING "" OFF) # prometheus-cpp

set(SENTRY_BACKEND none)

# Subdirectories
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/backtrace EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/libzmq EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/cppzmq EXCLUDE_FROM_ALL) # Should be after libzmq
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/prometheus-cpp EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/rapidjson EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/sentry EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/spdlog EXCLUDE_FROM_ALL)

# Enable test framework
if(XXX_BUILD_TESTS)
  set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
  include(CTest)
  enable_testing()

  option(BUILD_GMOCK OFF)
  add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()

# Set Targets
add_library("${PROJECT_NAME}-lib" SHARED ${ProjectSources})
set_target_properties(
  "${PROJECT_NAME}-lib"
  PROPERTIES OUTPUT_NAME "${PROJECT_NAME}"
             VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
             SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories("${PROJECT_NAME}-lib" PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries(
  "${PROJECT_NAME}-lib"
  stdc++fs
  spdlog
  cppzmq
  sentry
  pull)

add_executable("${PROJECT_NAME}" ${PROJECT_SOURCE_DIR}/src/main.cpp ${BACKWARD_ENABLE})
target_include_directories("${PROJECT_NAME}" PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries("${PROJECT_NAME}" "${PROJECT_NAME}-lib")
add_backward("${PROJECT_NAME}")

# Update build number at every build
add_custom_command(
  TARGET "${PROJECT_NAME}"
  PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -P "${PROJECT_SOURCE_DIR}/cmake/AutoBuildNumber.cmake"
  COMMENT "${BoldBlue}Updating build number${ColourReset}")

add_custom_target(
  package
  COMMAND rm -rf ${CMAKE_BINARY_DIR}/temp/*
  COMMAND mkdir -p ${CMAKE_BINARY_DIR}/temp
  COMMAND /bin/bash ${PROJECT_SOURCE_DIR}/scripts/ldd-copy-dependencies.sh
    -b ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/"${PROJECT_NAME}"
    -t ${CMAKE_BINARY_DIR}/temp
    # Regex to exclude system library path
    -r "\"^(/usr/lib64)|^(/usr/lib)|^(/lib64)|^(/lib)\""
  COMMAND /bin/bash ${PROJECT_SOURCE_DIR}/scripts/create_installer.sh
    -f ${CMAKE_BINARY_DIR}/temp
    -e ${PROJECT_SOURCE_DIR}/scripts/makeself/makeself.sh
    -v ${PROJECT_VERSION}
    -n "${PROJECT_NAME}"
  DEPENDS "${PROJECT_NAME}"
)

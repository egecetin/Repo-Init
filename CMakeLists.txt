cmake_minimum_required(VERSION 3.16)
project(XXX)

# CMake Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Colorize)
include(CompilerOptions)
include(CompilerFlags)
include(Doxy)
include(GitVersion)
include(GraphViz)
include(SvnVersion)

option(XXX_BUILD_TESTS "Build XXX tests" ON)
option(XXX_ENABLE_COVERAGE "Enables coverage report generation" OFF)
option(XXX_ENABLE_MEMLEAK_CHECK "Enables Memory Leak Checker for unit tests" OFF)

if(NOT DEFINED REVISION_VERSION)
  message(SEND_ERROR "Can't find SVN or Git. The folder is not under version control.")
endif()

# Set project properties
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)

if(EXISTS "BuildNumberCache.txt")
  file(READ "BuildNumberCache.txt" BUILD_NUMBER)
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
set(CMAKE_CXX_STANDARD 14)

configure_file(build_number.h.in "Version.h")

# Define sources
file(GLOB ProjectSources
  src/connection/Http.cpp
  src/connection/RawSocket.cpp
  src/connection/Zeromq.cpp
  src/logging/Logger.cpp
  src/logging/Loki.cpp
  src/logging/Sentry.cpp
  src/metrics/Performance.cpp
  src/metrics/PrometheusServer.cpp
  src/metrics/Status.cpp
  src/telnet/TelnetServer.cpp
  src/telnet/TelnetStats.cpp
  src/zeromq/ZeroMQServer.cpp
  src/zeromq/ZeroMQStats.cpp
  src/Control.cpp
  src/Utils.cpp
)

# Include directories
include_directories(include)

include_directories(thirdparty/cppzmq)
include_directories(thirdparty/date/include)
include_directories(thirdparty/http-status-codes)
include_directories(thirdparty/libzmq/include)
include_directories(thirdparty/prometheus-cpp/core/include)
include_directories(thirdparty/prometheus-cpp/pull/include)
include_directories(thirdparty/rapidjson/include)
include_directories(thirdparty/sentry/include)
include_directories(thirdparty/spdlog/include)
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

set(ENABLE_COMPRESSION OFF) # prometheus-cpp
set(SENTRY_BACKEND none)

# Subdirectories
add_subdirectory(thirdparty/backtrace EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/libzmq EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/cppzmq EXCLUDE_FROM_ALL) # Should be after libzmq
add_subdirectory(thirdparty/prometheus-cpp EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/rapidjson EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/sentry EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/spdlog EXCLUDE_FROM_ALL)

# Set Targets
add_library("${PROJECT_NAME}-lib" SHARED ${ProjectSources})
set_target_properties(
  "${PROJECT_NAME}-lib"
  PROPERTIES OUTPUT_NAME "${PROJECT_NAME}"
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
  SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories("${PROJECT_NAME}-lib" PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries(
  "${PROJECT_NAME}-lib" PUBLIC
  stdc++fs
  spdlog
  cppzmq
  sentry
  pull)

add_executable("${PROJECT_NAME}" src/main.cpp ${BACKWARD_ENABLE})
target_include_directories("${PROJECT_NAME}" PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries("${PROJECT_NAME}" PRIVATE "${PROJECT_NAME}-lib")
add_backward("${PROJECT_NAME}")

# Update build number at every build
add_custom_command(
  TARGET "${PROJECT_NAME}"
  PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AutoBuildNumber.cmake"
  COMMENT "${BoldBlue}Updating build number${ColourReset}")

# Enable test framework
if(XXX_BUILD_TESTS)
  set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
  include(CTest)
  enable_testing()

  add_subdirectory(tests)
endif()

if(XXX_ENABLE_COVERAGE)
  include(CodeCoverage)
  set(GCOVR_ADDITIONAL_ARGS "--exclude-throw-branches")
  setup_target_for_coverage_gcovr_html(
    NAME
    coverage
    EXECUTABLE
    ${TestExecutables}
    DEPENDENCIES
    ${TestExecutables}
    EXCLUDE
    "${PROJECT_SOURCE_DIR}/src/main.cpp"
    "${PROJECT_SOURCE_DIR}/thirdparty/*"
    "${PROJECT_SOURCE_DIR}/build/*"
    "${PROJECT_SOURCE_DIR}/tests/*")

  setup_target_for_coverage_gcovr_xml(
    NAME
    coverage-xml
    EXECUTABLE
    ${TestExecutables}
    DEPENDENCIES
    ${TestExecutables}
    EXCLUDE
    "${PROJECT_SOURCE_DIR}/src/main.cpp"
    "${PROJECT_SOURCE_DIR}/thirdparty/*"
    "${PROJECT_SOURCE_DIR}/build/*"
    "${PROJECT_SOURCE_DIR}/tests/*")
endif()

add_custom_target(
  makeself
  COMMAND rm -rf ${CMAKE_BINARY_DIR}/temp/*
  COMMAND mkdir -p ${CMAKE_BINARY_DIR}/temp
  COMMAND /bin/bash ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ldd-copy-dependencies.sh
  -b ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/"${PROJECT_NAME}"
  -t ${CMAKE_BINARY_DIR}/temp

  # Regex to exclude system library path
  -r "\"^(/usr/lib64)|^(/usr/lib)|^(/lib64)|^(/lib)\""
  COMMAND /bin/bash ${CMAKE_CURRENT_SOURCE_DIR}/scripts/create_installer.sh
  -f ${CMAKE_BINARY_DIR}/temp
  -e ${CMAKE_CURRENT_SOURCE_DIR}/scripts/makeself/makeself.sh
  -v ${PROJECT_VERSION}
  -n "${PROJECT_NAME}"
  DEPENDS "${PROJECT_NAME}"
)

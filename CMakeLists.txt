cmake_minimum_required(VERSION 3.16)
project(XXX)

# CMake Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(CompilerSecurityOptions)
include(Doxy)
include(ExternalProject)
include(GenerateSymbols)
include(GitVersion)
include(GraphViz)

find_package(CURL REQUIRED)
find_package(spdlog REQUIRED)
find_package(ZLIB REQUIRED)

option(XXX_BUILD_TESTS "Build XXX tests" ON)
option(XXX_BUILD_UNITTESTS "Build XXX unit tests" ON)
option(XXX_BUILD_FUZZTESTS "Build XXX fuzz tests" OFF)
option(XXX_ENABLE_COVERAGE "Enables coverage report generation" OFF)
option(XXX_ENABLE_MEMLEAK_CHECK "Enables Memory Leak Checker for unit tests" OFF)

# Set project properties
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION_TWEAK ${REVISION_VERSION})
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${REVISION_VERSION}")
message(STATUS "Compiling Version ${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Disable/Enable options of subprojects
set(CPPZMQ_BUILD_TESTS OFF CACHE BOOL "")

set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "")
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "")
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "")

set(ENABLE_TESTING OFF CACHE BOOL "") # prometheus-cpp
set(ENABLE_COMPRESSION OFF CACHE BOOL "") # prometheus-cpp

set(SENTRY_BACKEND none) # Disable backend to manually use crashpad

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
  ${PROJECT_SOURCE_DIR}/src/Tracer.cpp
  ${PROJECT_SOURCE_DIR}/src/Utils.cpp
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

# Subdirectories
add_subdirectory(thirdparty/cppzmq EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/prometheus-cpp EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/rapidjson EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/sentry EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/sentry/external/crashpad EXCLUDE_FROM_ALL)

# Add crashpad separately from sentry to enable minidump generation without sentry server
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/crashpad)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/crashpad/third_party/mini_chromium)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/crashpad/third_party/mini_chromium/mini_chromium)

# Add breakpad to compile dump_syms and minidump_stackwalk
file(COPY ${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/third_party
  DESTINATION ${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/breakpad/src/)

ExternalProject_Add(breakpad
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/breakpad
  BINARY_DIR ${CMAKE_BINARY_DIR}/breakpad_bin
  CONFIGURE_COMMAND "<SOURCE_DIR>/configure"
  BUILD_COMMAND make -j2
  INSTALL_COMMAND "")

# Set Targets
add_library(${PROJECT_NAME}-lib SHARED ${ProjectSources})
set_target_properties(
  ${PROJECT_NAME}-lib
  PROPERTIES OUTPUT_NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
  SOVERSION ${PROJECT_VERSION_MAJOR})
target_compile_options(${PROJECT_NAME}-lib PRIVATE -Wall -Wextra -g -Wl,--build-id)
target_include_directories(${PROJECT_NAME}-lib PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(
  ${PROJECT_NAME}-lib PUBLIC
  cppzmq
  crashpad::client
  CURL::libcurl
  prometheus-cpp::pull
  RapidJSON
  sentry::sentry
  spdlog::spdlog)
enable_security_flags_for_target(${PROJECT_NAME}-lib)
generate_symbols_for_target(${PROJECT_NAME}-lib)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -g -Wl,--build-id)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-lib)
enable_security_flags_for_target(${PROJECT_NAME})
generate_symbols_for_target(${PROJECT_NAME})
add_dependencies(${PROJECT_NAME} crashpad::handler)

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-lib)

# Enable test framework
if(XXX_BUILD_TESTS)
  if(XXX_ENABLE_COVERAGE)
    include(CodeCoverage)
    set(GCOVR_ADDITIONAL_ARGS "--exclude-throw-branches")
    append_coverage_compiler_flags()
  endif()

  set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
  include(CTest)
  enable_testing()

  add_subdirectory(tests)

  if(XXX_ENABLE_COVERAGE)
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
endif()

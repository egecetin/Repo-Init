cmake_minimum_required(VERSION 3.16)
project(XXX)

# CMake Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(CompilerOptions)
include(CompilerSecurityOptions)
include(Doxy)
include(GitVersion)
include(GraphViz)

option(XXX_BUILD_TESTS "Build XXX tests" ON)
option(XXX_BUILD_UNITTESTS "Build XXX unit tests" ON)
option(XXX_BUILD_FUZZTESTS "Build XXX fuzz tests" OFF)
option(XXX_ENABLE_COVERAGE "Enables coverage report generation" OFF)
option(XXX_ENABLE_MEMLEAK_CHECK "Enables Memory Leak Checker for unit tests" OFF)

find_package(CURL REQUIRED)

if(NOT DEFINED REVISION_VERSION)
  message(WARNING "Can't find Git. The folder is not under version control.")
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
message(STATUS "Compiling Version ${PROJECT_VERSION}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib/static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib/shared)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib/static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib/shared)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/bin)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Required for linking of crashpad_client
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -g -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -DNDEBUG")

# Disable/Enable options of subprojects
set(BUILD_SHARED_LIBS ON CACHE BOOL "")
set(BUILD_STATIC OFF CACHE BOOL "")
set(BUILD_TESTS OFF CACHE BOOL "")
set(SENTRY_BUILD_SHARED_LIBS ON CACHE BOOL "")
set(SPDLOG_BUILD_SHARED ON CACHE BOOL "")

set(WITH_PERF_TOOL OFF CACHE BOOL "")
set(WITH_DOCS OFF CACHE BOOL "")
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "")
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "")

set(CPPZMQ_BUILD_TESTS OFF CACHE BOOL "")
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "")
set(ENABLE_TESTING OFF CACHE BOOL "") # prometheus-cpp

set(ENABLE_COMPRESSION OFF CACHE BOOL "") # prometheus-cpp
set(SENTRY_BACKEND none) # Disable backend to manually use crashpad

# Set Compiler Properties
set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_CXX_STANDARD 14)

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
add_subdirectory(thirdparty/backtrace EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/libzmq EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/cppzmq EXCLUDE_FROM_ALL) # Should be after libzmq
add_subdirectory(thirdparty/prometheus-cpp EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/rapidjson EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/sentry EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/spdlog EXCLUDE_FROM_ALL)

# Add crashpad separately from sentry to enable minidump generation without sentry server
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/crashpad)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/crashpad/third_party/mini_chromium)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/crashpad/third_party/mini_chromium/mini_chromium)

add_subdirectory(thirdparty/sentry/external/crashpad EXCLUDE_FROM_ALL)

# Set Targets
add_library("${PROJECT_NAME}-lib" SHARED ${ProjectSources})
set_target_properties(
  "${PROJECT_NAME}-lib"
  PROPERTIES OUTPUT_NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
  SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories("${PROJECT_NAME}-lib" PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries(
  "${PROJECT_NAME}-lib" PUBLIC
  cppzmq
  crashpad_client
  CURL::libcurl
  prometheus-cpp::pull
  RapidJSON
  sentry::sentry
  spdlog::spdlog)
enable_security_flags_for_target("${PROJECT_NAME}-lib")

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/src/main.cpp ${BACKWARD_ENABLE})
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE "${PROJECT_NAME}-lib" Backward::Backward)
enable_security_flags_for_target(${PROJECT_NAME})

# Update build number at every build
add_custom_command(
  TARGET ${PROJECT_NAME}
  PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AutoBuildNumber.cmake"
  COMMENT "Updating build number")

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

add_custom_target(
  makeself
  COMMAND rm -rf ${CMAKE_BINARY_DIR}/temp/*
  COMMAND mkdir -p ${CMAKE_BINARY_DIR}/temp
  COMMAND /bin/bash ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ldd-copy-dependencies.sh
  -b ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/${PROJECT_NAME}
  -t ${CMAKE_BINARY_DIR}/temp

  # Regex to exclude system library path
  -r "\"^(/usr/lib64)|^(/usr/lib)|^(/lib64)|^(/lib)\""
  COMMAND /bin/bash ${CMAKE_CURRENT_SOURCE_DIR}/scripts/create_installer.sh
  -f ${CMAKE_BINARY_DIR}/temp
  -e ${CMAKE_CURRENT_SOURCE_DIR}/scripts/makeself/makeself.sh
  -v ${PROJECT_VERSION}
  -n ${PROJECT_NAME}
  DEPENDS ${PROJECT_NAME}
)

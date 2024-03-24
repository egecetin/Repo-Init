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
find_package(RapidJSON REQUIRED)
find_package(spdlog REQUIRED)
find_package(ZLIB REQUIRED)

option(XXX_BUILD_TESTS "Build XXX tests" ON)
option(XXX_BUILD_UNITTESTS "Build XXX unit tests" ON)
option(XXX_BUILD_FUZZTESTS "Build XXX fuzz tests" OFF)
option(XXX_ENABLE_COVERAGE "Enables coverage report generation" OFF)
option(XXX_ENABLE_MEMLEAK_CHECK "Enables Memory Leak Checker for unit tests" OFF)
option(XXX_ENABLE_SYMBOL_GENERATION "Enables symbol generation for binaries (Requires breakpad)" OFF)
option(XXX_ENABLE_PACKAGING "Enables RPM packaging" OFF)

# Set project properties
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION_TWEAK ${REVISION_VERSION})
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${REVISION_VERSION}")
message(STATUS "Compiling Version ${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Disable/Enable options of subprojects
set(CPPZMQ_BUILD_TESTS OFF CACHE BOOL "")
set(ENABLE_TESTING OFF CACHE BOOL "") # prometheus-cpp
set(INSTALL_GMOCK OFF CACHE BOOL "")
set(INSTALL_GTEST OFF CACHE BOOL "")

set(SENTRY_BACKEND none) # Disable backend to manually use crashpad

configure_file(Version.h.in "Version.h")

# Define sources
file(GLOB ProjectSources
  ${PROJECT_SOURCE_DIR}/src/connection/Http.cpp
  ${PROJECT_SOURCE_DIR}/src/connection/RawSocket.cpp
  ${PROJECT_SOURCE_DIR}/src/logging/Logger.cpp
  ${PROJECT_SOURCE_DIR}/src/logging/Loki.cpp
  ${PROJECT_SOURCE_DIR}/src/logging/Sentry.cpp
  ${PROJECT_SOURCE_DIR}/src/metrics/Performance.cpp
  ${PROJECT_SOURCE_DIR}/src/metrics/PrometheusServer.cpp
  ${PROJECT_SOURCE_DIR}/src/metrics/ProcessMetrics.cpp
  ${PROJECT_SOURCE_DIR}/src/metrics/Status.cpp
  ${PROJECT_SOURCE_DIR}/src/telnet/TelnetServer.cpp
  ${PROJECT_SOURCE_DIR}/src/telnet/TelnetStats.cpp
  ${PROJECT_SOURCE_DIR}/src/utils/ConfigParser.cpp
  ${PROJECT_SOURCE_DIR}/src/utils/Tracer.cpp
  ${PROJECT_SOURCE_DIR}/src/zeromq/ZeroMQ.cpp
  ${PROJECT_SOURCE_DIR}/src/zeromq/ZeroMQMonitor.cpp
  ${PROJECT_SOURCE_DIR}/src/zeromq/ZeroMQServer.cpp
  ${PROJECT_SOURCE_DIR}/src/zeromq/ZeroMQStats.cpp
)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(${PROJECT_SOURCE_DIR}/thirdparty/cppzmq)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/date/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/http-status-codes)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/prometheus-cpp/core/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/prometheus-cpp/pull/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/include)

# Subdirectories
add_subdirectory(thirdparty/cppzmq EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/prometheus-cpp EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/sentry EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/sentry/external/crashpad EXCLUDE_FROM_ALL)

# Add crashpad separately from sentry to enable minidump generation without sentry server
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/crashpad)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/crashpad/third_party/mini_chromium)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/crashpad/third_party/mini_chromium/mini_chromium)

# Add breakpad to compile dump_syms and minidump_stackwalk
if(XXX_ENABLE_SYMBOL_GENERATION)
  file(COPY ${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/third_party
    DESTINATION ${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/breakpad/src/)

  ExternalProject_Add(breakpad
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/thirdparty/sentry/external/breakpad
    BINARY_DIR ${CMAKE_BINARY_DIR}/breakpad_bin
    CONFIGURE_COMMAND "<SOURCE_DIR>/configure"
    BUILD_COMMAND make -j2 # Prevent OOM for the virtual machines when parallel compilation invoked
    INSTALL_COMMAND "")
endif()

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
  sentry::sentry
  spdlog::spdlog)
enable_security_flags_for_target(${PROJECT_NAME}-lib)

if(XXX_ENABLE_SYMBOL_GENERATION)
  generate_symbols_for_target(${PROJECT_NAME}-lib)
endif()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -g -Wl,--build-id)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-lib)
enable_security_flags_for_target(${PROJECT_NAME})

if(XXX_ENABLE_SYMBOL_GENERATION)
  generate_symbols_for_target(${PROJECT_NAME})
endif()

add_dependencies(${PROJECT_NAME} crashpad::handler)

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-lib crashpad_handler)

set(CONFIG_BASE_DIR "${CMAKE_BINARY_DIR}")
configure_file(config.json.in "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json")
file(MAKE_DIRECTORY ${CONFIG_BASE_DIR}/share)

set(CONFIG_BASE_DIR "${CMAKE_INSTALL_PREFIX}")
configure_file(config.json.in "${CMAKE_BINARY_DIR}/config.json")
install(FILES ${CMAKE_BINARY_DIR}/config.json DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})

if(XXX_ENABLE_PACKAGING)
  set(EXE_PATH ${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME})
  set(SERVICE_NAME ${PROJECT_NAME}.service)
  set(POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/postinst.sh)
  set(POST_UNINSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/postuninst.sh)
  set(PRE_UNINSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/preuninst.sh)

  configure_file(${PROJECT_SOURCE_DIR}/scripts/template.service.in ${CMAKE_CURRENT_BINARY_DIR}/${SERVICE_NAME} @ONLY)
  configure_file(${PROJECT_SOURCE_DIR}/scripts/post.sh.in ${POST_INSTALL_SCRIPT})
  configure_file(${PROJECT_SOURCE_DIR}/scripts/postun.sh.in ${POST_UNINSTALL_SCRIPT})
  configure_file(${PROJECT_SOURCE_DIR}/scripts/preun.sh.in ${PRE_UNINSTALL_SCRIPT})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.service DESTINATION ${CMAKE_INSTALL_PREFIX}/share)

  set(CPACK_GENERATOR "RPM")
  set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
  set(CPACK_PACKAGE_RELEASE 1)
  set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
  set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
  set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
  set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
  set(CPACK_RPM_PACKAGE_LICENSE "MIT")
  set(CPACK_RPM_PACKAGE_REQUIRES_POSTUN "systemd")
  set(CPACK_RPM_PACKAGE_REQUIRES_PREUN "systemd")
  set(CPACK_RPM_PACKAGE_REQUIRES_POST "systemd")
  set(CPACK_RPM_PACKAGE_REQUIRES "libcurl >= 7.0, spdlog >= 1.8.0, zeromq >= 4.0.0")
  set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE ${POST_INSTALL_SCRIPT})
  set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE ${POST_UNINSTALL_SCRIPT})
  set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE ${PRE_UNINSTALL_SCRIPT})
  include(CPack)
endif()

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
      "${PROJECT_SOURCE_DIR}/tests/unittests/*"
      "${PROJECT_SOURCE_DIR}/tests/fuzztests/*")

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
      "${PROJECT_SOURCE_DIR}/tests/unittests/*"
      "${PROJECT_SOURCE_DIR}/tests/fuzztests/*")
  endif()
endif()

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
include(Sanitizers)
include(SvnVersion)

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
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${BUILD_NUMBER}-rev${REVISION_VERSION}")
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
    ${PROJECT_SOURCE_DIR}/src/connection/Zeromq.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/Sentry.cpp
    ${PROJECT_SOURCE_DIR}/src/metrics/Performance.cpp
    ${PROJECT_SOURCE_DIR}/src/metrics/Reporter.cpp
    ${PROJECT_SOURCE_DIR}/src/metrics/Status.cpp
    ${PROJECT_SOURCE_DIR}/src/telnet/TelnetServer.cpp
    ${PROJECT_SOURCE_DIR}/src/Control.cpp
    ${PROJECT_SOURCE_DIR}/src/Utils.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    )

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(${PROJECT_SOURCE_DIR}/thirdparty/libzmq)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/cppzmq)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/rapidjson/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/spdlog/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/prometheus-cpp/core/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/prometheus-cpp/pull/include)

# Disable/Enable options of subprojects
option(BUILD_TESTS "" OFF) # libzmq
option(WITH_DOCS "" OFF) # libzmq
option(CPPZMQ_BUILD_TESTS "" OFF)
option(RAPIDJSON_BUILD_DOC "" OFF)
option(RAPIDJSON_BUILD_EXAMPLES "" OFF)
option(RAPIDJSON_BUILD_TESTS "" OFF)
option(SPDLOG_BUILD_SHARED "" ON)
option(SENTRY_BUILD_SHARED_LIBS "" ON)
option(BUILD_SHARED_LIBS "" ON) # prometheus-cpp
option(ENABLE_COMPRESSION "" OFF) # prometheus-cpp
option(ENABLE_TESTING "" OFF) # prometheus-cpp
set(SENTRY_BACKEND none)

# Subdirectories
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/libzmq EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/cppzmq EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/rapidjson EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/sentry EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/prometheus-cpp EXCLUDE_FROM_ALL)

# Enable test framework
include(CTest)
enable_testing()

add_subdirectory(${PROJECT_SOURCE_DIR}/tests)

# Set Targets
add_library("${PROJECT_NAME}_Lib_Shared" SHARED ${ProjectSources})
set_target_properties(
  "${PROJECT_NAME}_Lib_Shared"
  PROPERTIES OUTPUT_NAME "${PROJECT_NAME}"
             VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
             SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories("${PROJECT_NAME}_Lib_Shared" PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries(
  "${PROJECT_NAME}_Lib_Shared"
  stdc++fs
  spdlog
  libzmq
  sentry
  core
  pull)

add_executable("${PROJECT_NAME}" ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_include_directories("${PROJECT_NAME}" PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries("${PROJECT_NAME}" "${PROJECT_NAME}_Lib_Shared")
add_sanitizers("${PROJECT_NAME}")

# Set install targets
install(
  TARGETS "${PROJECT_NAME}"
          "${PROJECT_NAME}_Lib_Shared"
          spdlog
          libzmq
          sentry
          core
          pull
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib)

# Update build number at every build
add_custom_command(
  TARGET "${PROJECT_NAME}_Lib_Shared"
  PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -P "${PROJECT_SOURCE_DIR}/cmake/AutoBuildNumber.cmake"
  COMMENT "${BoldBlue}Updating build number${ColourReset}")

# CPack
if(${CMAKE_VERSION} VERSION_LESS "3.21.0")
  message("${BoldYellow}Please consider to switch to CMake 3.21.0 or newer to enable CPack support${ColourReset}")
else()
  set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
  set(CPACK_GENERATOR "RPM")
  set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
  set(CPACK_PACKAGE_RELEASE ${PROJECT_VERSION_PATCH})
  set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CMAKE_SYSTEM_PROCESSOR}")
  set(CPACK_RPM_SPEC_MORE_DEFINE "%define _build_id_links none")
  set(CPACK_OUTPUT_FILE_PREFIX ${PROJECT_SOURCE_DIR}/dist/packages)
  set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE)
  set(CPACK_RESOURCE_FILE_README ${PROJECT_SOURCE_DIR}/README.md)
  set(CPACK_PACKAGE_CONTACT "Humanity")
  list(APPEND CPACK_POST_BUILD_SCRIPTS "${CMAKE_MODULE_PATH}/PostBuild.cmake")
  include(CPack)

  # Cache variables for scripts
  set(CacheForScript ${CMAKE_MODULE_PATH}/CMakeCacheForScript.cmake)
  file(WRITE ${CacheForScript} "")

  get_cmake_property(Vars VARIABLES)
  foreach(Var ${Vars})
    if(${Var})
      string(REPLACE "\\" "\\\\" ${Var} "${${Var}}")
      string(REPLACE "\"" "\\\"" ${Var} "${${Var}}")
    endif()
    file(APPEND ${CacheForScript} "set(${Var} \"${${Var}}\")\n")
  endforeach()
endif()

cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

# Enable gcov if Debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  include(CodeCoverage)
  set(GCOVR_ADDITIONAL_ARGS "--exclude-throw-branches")
  setup_target_for_coverage_gcovr_html(
    NAME
    coverage
    EXECUTABLE
    TestMain
    DEPENDENCIES
    TestMain
    EXCLUDE
    "${PROJECT_SOURCE_DIR}/src/main.cpp"
    "${PROJECT_SOURCE_DIR}/thirdparty/*"
    "${PROJECT_SOURCE_DIR}/build/*"
    "${PROJECT_SOURCE_DIR}/tests/*")

  setup_target_for_coverage_gcovr_xml(
    NAME
    coverage-xml
    EXECUTABLE
    TestMain
    DEPENDENCIES
    TestMain
    EXCLUDE
    "${PROJECT_SOURCE_DIR}/src/main.cpp"
    "${PROJECT_SOURCE_DIR}/thirdparty/*"
    "${PROJECT_SOURCE_DIR}/build/*"
    "${PROJECT_SOURCE_DIR}/tests/*")
endif()

# Add sources
file(GLOB ProjectTestSources
    ${PROJECT_SOURCE_DIR}/tests/Connection_Tests.cpp
    ${PROJECT_SOURCE_DIR}/tests/Metrics_Tests.cpp
    ${PROJECT_SOURCE_DIR}/tests/Telnet_Tests.cpp
    ${PROJECT_SOURCE_DIR}/tests/Utils_Tests.cpp
    ${PROJECT_SOURCE_DIR}/tests/ZeroMQ_Tests.cpp
    ${PROJECT_SOURCE_DIR}/tests/gtest_main.cpp
    )

configure_file(test-static-definitions.h.in test-static-definitions.h)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/tests/MemPlumber)
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(${PROJECT_SOURCE_DIR}/tests/MemPlumber)
add_subdirectory(${PROJECT_SOURCE_DIR}/tests/gtest)

# Test cases
add_executable(TestMain ${ProjectTestSources})
target_include_directories(TestMain PRIVATE "${PROJECT_BINARY_DIR}/tests")
target_link_libraries(TestMain gtest_main memplumber "${PROJECT_NAME}-static")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(TestMain PUBLIC --coverage)
  target_link_libraries(TestMain gcov)
endif()

# Enum tests
include(GoogleTest)
gtest_discover_tests(TestMain)

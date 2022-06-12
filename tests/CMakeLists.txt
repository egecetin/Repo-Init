cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

# Include Google Test
include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable gcov if Debug
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	include(CodeCoverage)
	setup_target_for_coverage_gcovr_html(NAME coverage
										EXECUTABLE TestMain
										DEPENDENCIES TestMain
										EXCLUDE "${PROJECT_SOURCE_DIR}/src/main.cpp" "${PROJECT_SOURCE_DIR}/thirdparty/*" "${PROJECT_SOURCE_DIR}/build/*" "${PROJECT_SOURCE_DIR}/tests/*")
endif()

# Enable test framework
include(CTest)
enable_testing()

# Add sources
FILE(GLOB ProjectTestSources
		${PROJECT_SOURCE_DIR}/tests/*.cpp
		${PROJECT_SOURCE_DIR}/tests/**/*.cpp
)

configure_file(test-static-definitions.h.in test-static-definitions.h)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/telnetserver/include)
include_directories(${PROJECT_SOURCE_DIR}/tests/MemPlumber)
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(${PROJECT_SOURCE_DIR}/tests/MemPlumber)

# Test cases
add_executable(TestMain ${ProjectTestSources})
target_include_directories(TestMain PRIVATE "${PROJECT_BINARY_DIR}/tests")
target_link_libraries(TestMain gtest_main memplumber XXX_Lib)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_options(TestMain PUBLIC --coverage)
	target_link_libraries(TestMain gcov)
endif()

# Enum tests
include(GoogleTest)
gtest_discover_tests(TestMain)

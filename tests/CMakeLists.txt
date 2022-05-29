cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 11)

# Include Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(CTest)
enable_testing()

FILE(GLOB ProjectTestSources
                ${PROJECT_SOURCE_DIR}/tests/*.cpp
                ${PROJECT_SOURCE_DIR}/tests/*.c
                ${PROJECT_SOURCE_DIR}/tests/**/*.cpp
                ${PROJECT_SOURCE_DIR}/tests/**/*.c)

# Test cases
add_executable(TestMain ${ProjectTestSources})
target_link_libraries(TestMain gtest_main XXX_Lib)

include(GoogleTest)
gtest_discover_tests(TestMain)

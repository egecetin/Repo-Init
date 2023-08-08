# Check compiler
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(SEND_ERROR "Fuzz testing not supported for ${CMAKE_CXX_COMPILER_ID} compiler")
endif()

# Add sources
file(GLOB ProjectFuzzTestSources
    test_fuzzer.cpp)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(FuzzTestMain ${ProjectFuzzTestSources})
target_include_directories(FuzzTestMain PRIVATE "${PROJECT_BINARY_DIR}/tests")
target_link_libraries(FuzzTestMain PRIVATE "${PROJECT_NAME}-lib")

target_compile_options(FuzzTestMain PRIVATE "-fsanitize=fuzzer")
target_link_options(FuzzTestMain PRIVATE "-fsanitize=fuzzer")

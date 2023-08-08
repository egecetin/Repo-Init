# Check compiler
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(SEND_ERROR "Fuzz testing not supported for ${CMAKE_CXX_COMPILER_ID} compiler")
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/tests)

# Fuzz sources
set(ProjectFuzzTestSources "\
test_fuzzer;")

# Create fuzz targets
foreach(target ${ProjectFuzzTestSources})
  add_executable(${target} ${target}.cpp)
  target_link_libraries(${target} PRIVATE "${PROJECT_NAME}-lib")
  target_compile_options(${target} PRIVATE "-fsanitize=fuzzer")
  target_link_options(${target} PRIVATE "-fsanitize=fuzzer")
  
  add_test(NAME ${target} COMMAND ${target})
endforeach(target ${ProjectFuzzTestSources})

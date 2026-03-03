if(XXX_ENABLE_MEMLEAK_CHECK)
  include_directories(thirdparty/MemPlumber)
  add_subdirectory(thirdparty/MemPlumber)

  # Disable MemPlumber tests from parent
  add_custom_target(
    ExcludeMemPlumberTests
    ALL
    COMMAND rm -f "${PROJECT_BINARY_DIR}/tests/thirdparty/MemPlumber/CTestTestfile.cmake"
  )
endif()

configure_file(test-static-definitions.h.in test-static-definitions.h)

option(XXX_ENABLE_ASAN "Enables Address Sanitizer for XXX" OFF)
option(XXX_ENABLE_MSAN "Enables Memory Sanitizer for XXX" OFF)
option(XXX_ENABLE_USAN "Enables Undefined Behaviour Sanitizer for XXX" OFF)
option(XXX_ENABLE_TSAN "Enables Thread Sanitizer for XXX" OFF)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/tests/include)

add_subdirectory(thirdparty/fuzztest)

file(
  GLOB ProjectTestSources
  src/Connection_UnitTests.cpp
  src/Logger_UnitTests.cpp
  src/Metrics_UnitTests.cpp
  src/Telnet_UnitTests.cpp
  src/Utils_UnitTests.cpp
  src/ZeroMQ_UnitTests.cpp
  src/gtest_main.cpp
)

# Sanitizers
if(XXX_ENABLE_ASAN)
  message(STATUS "Enabling Address Sanitizer")
  target_compile_options(
    "${PROJECT_NAME}-lib"
    PUBLIC "-fsanitize=address -fsanitize-coverage=inline-8bit-counters -fsanitize-coverage=trace-cmp"
  )
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=address")
endif()

if(XXX_ENABLE_MSAN)
  message(STATUS "Enabling Memory Sanitizer")
  target_compile_options(
    "${PROJECT_NAME}-lib"
    PUBLIC "-fsanitize=memory -fsanitize-coverage=inline-8bit-counters -fsanitize-coverage=trace-cmp"
  )
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=memory")
endif()

if(XXX_ENABLE_USAN)
  message(STATUS "Enabling Undefined Behaviour Sanitizer")
  target_compile_options(
    "${PROJECT_NAME}-lib"
    PUBLIC "-fsanitize=undefined -fsanitize-coverage=inline-8bit-counters -fsanitize-coverage=trace-cmp"
  )
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=undefined")
endif()

if(XXX_ENABLE_TSAN)
  message(STATUS "Enabling Thread Sanitizer")
  target_compile_options(
    "${PROJECT_NAME}-lib"
    PUBLIC "-fsanitize=thread -fsanitize-coverage=inline-8bit-counters -fsanitize-coverage=trace-cmp"
  )
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=thread")
endif()

add_executable(TestMain ${ProjectTestSources})
target_include_directories(TestMain PRIVATE "${PROJECT_BINARY_DIR}/tests")
target_link_libraries(TestMain PRIVATE "${PROJECT_NAME}-lib")
link_fuzztest(TestMain)

if(XXX_ENABLE_MEMLEAK_CHECK)
  target_link_libraries(TestMain PRIVATE memplumber)
endif()

include(GoogleTest)
gtest_discover_tests(TestMain)

set(TestExecutables TestMain PARENT_SCOPE)

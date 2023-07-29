configure_file(test-static-definitions.h.in test-static-definitions.h)

# Test options
option(XXX_BUILD_UNITTESTS "Build XXX unit tests" ON)
option(XXX_BUILD_FUZZTESTS "Build XXX fuzz tests" OFF)

option(XXX_ENABLE_ASAN "Enables Address Sanitizer for XXX" ON)
option(XXX_ENABLE_MSAN "Enables Memory Sanitizer for XXX" OFF)
option(XXX_ENABLE_USAN "Enables Undefined Behaviour Sanitizer for XXX" ON)
option(XXX_ENABLE_TSAN "Enables Thread Sanitizer for XXX" OFF)

if(XXX_BUILD_UNITTESTS AND XXX_BUILD_FUZZTESTS)
  message(SEND_ERROR "Compiling of unit and fuzz tests at the same is not supported")
endif()

# Sanitizers
if(XXX_ENABLE_ASAN)
  target_compile_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=address")
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=address")
endif()

if(XXX_ENABLE_MSAN)
  target_compile_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=memory")
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=memory")
endif()

if(XXX_ENABLE_USAN)
  target_compile_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=undefined")
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=undefined")
endif()

if(XXX_ENABLE_TSAN)
  target_compile_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=thread")
  target_link_options("${PROJECT_NAME}-lib" PUBLIC "-fsanitize=thread")
endif()

# Add sources
file(GLOB ProjectUnitTestSources
  Connection_UnitTests.cpp
  Metrics_UnitTests.cpp
  Telnet_UnitTests.cpp
  Utils_UnitTests.cpp
  ZeroMQ_UnitTests.cpp
  gtest_main.cpp
)

file(GLOB ProjectFuzzTestSources
  Fuzztest_Test.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include)

if(XXX_ENABLE_MEMLEAK_CHECK)
  include_directories(MemPlumber)
  add_subdirectory(MemPlumber)

  # Disable MemPlumber tests from parent
  add_custom_target(ExcludeMemPlumberTests ALL
    COMMAND rm -f "${PROJECT_BINARY_DIR}/tests/MemPlumber/CTestTestfile.cmake")
endif()

include(GoogleTest)

if(XXX_BUILD_UNITTESTS)
  add_subdirectory(gtest)

  add_executable(UnitTestMain ${ProjectUnitTestSources})
  target_include_directories(UnitTestMain PRIVATE "${PROJECT_BINARY_DIR}/tests")
  target_link_libraries(UnitTestMain PRIVATE gtest_main "${PROJECT_NAME}-lib" curl)

  if(XXX_ENABLE_MEMLEAK_CHECK)
    target_link_libraries(UnitTestMain PRIVATE memplumber)
  endif()

  gtest_discover_tests(UnitTestMain)
  list(APPEND TestExecutablesLocal UnitTestMain)
endif()

if(XXX_BUILD_FUZZTESTS)
  set(CMAKE_CXX_STANDARD 17) # fuzztest requires C++17
  option(FUZZTEST_FUZZING_MODE "" ON)

  add_subdirectory(fuzztest)

  add_executable(FuzzTestMain ${ProjectFuzzTestSources})
  link_fuzztest(FuzzTestMain)
  target_link_libraries(FuzzTestMain PRIVATE "${PROJECT_NAME}-lib")

  gtest_discover_tests(FuzzTestMain)
  list(APPEND TestExecutablesLocal FuzzTestMain)
endif()

set(TestExecutables ${TestExecutablesLocal} PARENT_SCOPE)

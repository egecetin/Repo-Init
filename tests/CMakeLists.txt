configure_file(test-static-definitions.h.in test-static-definitions.h)

option(XXX_BUILD_UNITTESTS "Build XXX unit tests" ON)
option(XXX_BUILD_FUZZTESTS "Build XXX fuzz tests" OFF)

# Add sources
file(GLOB ProjectUnitTestSources
  Connection_Tests.cpp
  Metrics_Tests.cpp
  Telnet_Tests.cpp
  Utils_Tests.cpp
  ZeroMQ_Tests.cpp
  gtest_main.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(gtest)

if(XXX_ENABLE_MEMLEAK_CHECK)
  include_directories(MemPlumber)
  add_subdirectory(MemPlumber)

  # Disable MemPlumber tests from parent
  add_custom_target(ExcludeMemPlumberTests ALL
    COMMAND rm -f "${PROJECT_BINARY_DIR}/tests/MemPlumber/CTestTestfile.cmake")
endif()

include(GoogleTest)

if(XXX_BUILD_UNITTESTS)
  add_executable(UnitTestMain ${ProjectUnitTestSources})
  target_include_directories(UnitTestMain PRIVATE "${PROJECT_BINARY_DIR}/tests")
  target_link_libraries(UnitTestMain gtest_main "${PROJECT_NAME}-lib" curl)

  if(XXX_ENABLE_MEMLEAK_CHECK)
    target_link_libraries(UnitTestMain memplumber)
  endif()
  gtest_discover_tests(UnitTestMain)

  list(APPEND TestExecutablesLocal UnitTestMain)
endif()

if(XXX_BUILD_FUZZTESTS)
endif()

set(TestExecutables ${TestExecutablesLocal} PARENT_SCOPE)

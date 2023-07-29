configure_file(test-static-definitions.h.in test-static-definitions.h)

option(XXX_BUILD_UNITTESTS "Build XXX unit tests" ON)
option(XXX_BUILD_FUZZTESTS "Build XXX fuzz tests" OFF)

# Add sources
file(GLOB ProjectUnitTestSources
  Connection_UnitTests.cpp
  Metrics_UnitTests.cpp
  Telnet_UnitTests.cpp
  Utils_UnitTests.cpp
  ZeroMQ_UnitTests.cpp
  gtest_main.cpp
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
  include_directories(${GTEST_INCLUDE_DIRS})
  add_subdirectory(gtest)

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
  if(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    set(COMPILER_CLANG 1) # Safe to treat AppleClang as a regular Clang, in general.
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(COMPILER_CLANG 1)
  else()
    message (FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} is not supported")
  endif()

  include(ExternalProject)
  ExternalProject_Add(
    fuzztest
    GIT_REPOSITORY "https://github.com/google/fuzztest.git"
    GIT_TAG main
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  )
endif()

set(TestExecutables ${TestExecutablesLocal} PARENT_SCOPE)

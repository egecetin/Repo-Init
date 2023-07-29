configure_file(test-static-definitions.h.in test-static-definitions.h)

# Test options
option(XXX_BUILD_UNITTESTS "Build XXX unit tests" ON)
option(XXX_BUILD_FUZZTESTS "Build XXX fuzz tests" OFF)

if(XXX_BUILD_UNITTESTS AND XXX_BUILD_FUZZTESTS)
  message(SEND_ERROR "Compiling of unit and fuzz tests at the same is not supported")
endif()

if(XXX_BUILD_UNITTESTS)
  if(XXX_ENABLE_MEMLEAK_CHECK)
    include_directories(MemPlumber)
    add_subdirectory(MemPlumber)

    # Disable MemPlumber tests from parent
    add_custom_target(ExcludeMemPlumberTests ALL
      COMMAND rm -f "${PROJECT_BINARY_DIR}/tests/MemPlumber/CTestTestfile.cmake")
  endif()

  add_subdirectory(gtest)
  add_subdirectory(unittests)
  list(APPEND TestExecutablesLocal UnitTestMain)
endif()

if(XXX_BUILD_FUZZTESTS)
  set(CMAKE_CXX_STANDARD 17) # fuzztest requires C++17
  option(FUZZTEST_FUZZING_MODE "" ON)

  add_subdirectory(google-fuzztest)
  add_subdirectory(fuzztests)
  list(APPEND TestExecutablesLocal FuzzTestMain)
endif()

set(TestExecutables ${TestExecutablesLocal} PARENT_SCOPE)

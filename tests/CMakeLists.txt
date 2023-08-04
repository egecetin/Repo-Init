
add_subdirectory(gtest)

if(XXX_ENABLE_MEMLEAK_CHECK)
  include_directories(MemPlumber)
  add_subdirectory(MemPlumber)

  # Disable MemPlumber tests from parent
  add_custom_target(ExcludeMemPlumberTests ALL
    COMMAND rm -f "${PROJECT_BINARY_DIR}/tests/MemPlumber/CTestTestfile.cmake")
endif()

configure_file(test-static-definitions.h.in test-static-definitions.h)

if(XXX_BUILD_UNITTESTS)
  add_subdirectory(unittests)
  list(APPEND TestExecutablesLocal UnitTestMain)
endif()

set(TestExecutables ${TestExecutablesLocal} PARENT_SCOPE)

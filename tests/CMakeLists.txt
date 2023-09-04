
if(XXX_ENABLE_MEMLEAK_CHECK)
  include_directories(MemPlumber)
  add_subdirectory(MemPlumber)

  # Disable MemPlumber tests from parent
  add_custom_target(ExcludeMemPlumberTests ALL
    COMMAND rm -f "${PROJECT_BINARY_DIR}/tests/MemPlumber/CTestTestfile.cmake")
endif()

configure_file(test-static-definitions.h.in test-static-definitions.h)

option(XXX_ENABLE_ASAN "Enables Address Sanitizer for XXX" OFF)
option(XXX_ENABLE_MSAN "Enables Memory Sanitizer for XXX" OFF)
option(XXX_ENABLE_USAN "Enables Undefined Behaviour Sanitizer for XXX" OFF)
option(XXX_ENABLE_TSAN "Enables Thread Sanitizer for XXX" OFF)

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

if(XXX_BUILD_UNITTESTS)
  add_subdirectory(gtest)
  add_subdirectory(unittests)
  list(APPEND TestExecutablesLocal UnitTestMain)
endif()

if(XXX_BUILD_FUZZTESTS)
  add_subdirectory(fuzztests)
  list(APPEND TestExecutablesLocal ${FuzzTestExecutables})
endif()

set(TestExecutables ${TestExecutablesLocal} PARENT_SCOPE)

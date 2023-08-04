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

option(XXX_ENABLE_ASAN "Enables Address Sanitizer for XXX" ON)
option(XXX_ENABLE_MSAN "Enables Memory Sanitizer for XXX" OFF)
option(XXX_ENABLE_USAN "Enables Undefined Behaviour Sanitizer for XXX" ON)
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

add_executable(UnitTestMain ${ProjectUnitTestSources})
target_include_directories(UnitTestMain PRIVATE "${PROJECT_BINARY_DIR}/tests")
target_link_libraries(UnitTestMain PRIVATE gtest_main "${PROJECT_NAME}-lib" curl)

if(XXX_ENABLE_MEMLEAK_CHECK)
    target_link_libraries(UnitTestMain PRIVATE memplumber)
endif()

include(GoogleTest)
gtest_discover_tests(UnitTestMain)

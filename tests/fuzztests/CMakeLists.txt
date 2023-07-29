
file(GLOB ProjectFuzzTestSources
    Fuzztest_Test.cpp
)

add_executable(FuzzTestMain ${ProjectFuzzTestSources})
link_fuzztest(FuzzTestMain)
target_link_libraries(FuzzTestMain PRIVATE "${PROJECT_NAME}-lib")

include(GoogleTest)
gtest_discover_tests(FuzzTestMain)

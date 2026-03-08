file(
  GLOB ProjectBenchmarkSources
  Hasher_Benchmarks.cpp
  Http_Benchmarks.cpp
  Telnet_Benchmarks.cpp
  benchmark_main.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/tests/include)

add_subdirectory(benchmark)
add_executable(BenchmarkMain ${ProjectBenchmarkSources})
target_link_libraries(BenchmarkMain PRIVATE "${PROJECT_NAME}-lib" benchmark::benchmark)

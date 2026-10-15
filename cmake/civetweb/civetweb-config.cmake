# Stub package config for the civetweb submodule built through add_subdirectory().
# The civetweb::civetweb and civetweb::civetweb-cpp targets are defined as aliases in the top level CMakeLists.txt,
# so find_package(civetweb CONFIG) from thirdparty libraries (e.g. prometheus-cpp) only needs to succeed.
set(civetweb_FOUND TRUE)

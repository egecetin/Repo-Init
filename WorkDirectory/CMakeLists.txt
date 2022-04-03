project(XXX)
cmake_minimum_required(VERSION 3.23)

# CMake Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(GitVersion)
include(SvnVersion)
include(AutoBuildNumber)

if(NOT DEFINED REVISION_VERSION)
    message(SEND_ERROR "Can't find SVN or Git. The folder is not under version control.")
endif()

# Set properties
set(CMAKE_PROJECT_VERSION 1.0.${BUILD_VERSION}.${REVISION_VERSION})
message(INFO " Compiling Version ${CMAKE_PROJECT_VERSION}")




# Update build number at every build
add_custom_command(
    TARGET XXX
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND}
        -P "AutoBuildNumber.cmake"
)


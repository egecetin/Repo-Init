project(TEST)
cmake_minimum_required(VERSION 3.23)

# CMake Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(git_version)
include(svn_version)

if(NOT DEFINED ${REVISION_VERSION})
    message(SEND_ERROR "Can't find SVN or Git. The folder is not under version control.")
endif()

# Set properties
set(CMAKE_PROJECT_VERSION 1.0.${REVISION_VERSION})
message(INFO "Compiling Version ${REVISION_VERSION}")


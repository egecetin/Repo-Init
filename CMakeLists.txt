cmake_minimum_required(VERSION 3.21)
project(XXX)

# CMake Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(Colorize)
include(GitVersion)
include(SvnVersion)
include(Doxy)

if(NOT DEFINED REVISION_VERSION)
	message(SEND_ERROR "Can't find SVN or Git. The folder is not under version control.")
endif()

# Set project properties
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)

if(EXISTS "${PROJECT_SOURCE_DIR}/BuildNumberCache.txt")
	file(READ "${PROJECT_SOURCE_DIR}/BuildNumberCache.txt" BUILD_NUMBER)
else()
	set(BUILD_NUMBER "1")
endif()

set(PROJECT_VERSION_PATCH ${BUILD_NUMBER})
set(PROJECT_VERSION_TWEAK ${REVISION_VERSION})
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${BUILD_NUMBER}-rev${REVISION_VERSION}")
message("${BoldBlue}Compiling Version ${PROJECT_VERSION}${ColourReset}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib/static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib/shared)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib/static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib/shared)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/bin)

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -DNDEBUG -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG -O3")

if(CMAKE_C_COMPILER STREQUAL "icc")
	message("${BoldBlue}Intel C Compiler detected. Updating flags ...${ColourReset}")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -xHost -use-intel-optimized-headers -static-intel")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -xHost -ipo -use-intel-optimized-headers -static-intel")
endif()

if(CMAKE_CXX_COMPILER STREQUAL "icpc")
	message("${BoldBlue}Intel C++ Compiler detected. Updating flags ...${ColourReset}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -xHost -use-intel-optimized-headers -static-intel")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -xHost -ipo -use-intel-optimized-headers -static-intel")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} --coverage")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} --coverage")
endif()

configure_file(build_number.h.in XXX_Version.h)

# Sources
FILE(GLOB ProjectSources
	${PROJECT_SOURCE_DIR}/src/*.cpp
	${PROJECT_SOURCE_DIR}/src/*.c
	${PROJECT_SOURCE_DIR}/src/**/*.cpp
	${PROJECT_SOURCE_DIR}/src/**/*.c)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(${PROJECT_SOURCE_DIR}/thirdparty/libzmq)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/cppzmq)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/rapidjson/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/spdlog/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/sentry/include)

# Disable/Enable options of subprojects
option(BUILD_TESTS "" OFF) # libzmq
option(WITH_DOCS "" OFF) # libzmq
option(CPPZMQ_BUILD_TESTS "" OFF)
option(RAPIDJSON_BUILD_DOC "" OFF)
option(RAPIDJSON_BUILD_EXAMPLES "" OFF)
option(RAPIDJSON_BUILD_TESTS "" OFF)
option(SPDLOG_BUILD_SHARED "" ON)
option(SENTRY_BUILD_SHARED_LIBS "" ON)

# Subdirectories
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/libzmq EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/cppzmq EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/rapidjson EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/sentry EXCLUDE_FROM_ALL)

# Add tests
add_subdirectory(${PROJECT_SOURCE_DIR}/tests EXCLUDE_FROM_ALL)

# Set Targets
add_library(XXX_Lib_Shared SHARED ${ProjectSources})
set_target_properties(
	XXX_Lib_Shared PROPERTIES
	OUTPUT_NAME "XXX"
	VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_compile_options(XXX_Lib_Shared PRIVATE -Wall -Wextra -Werror -pedantic)
target_include_directories(XXX_Lib_Shared PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries(XXX_Lib_Shared stdc++fs spdlog libzmq sentry)

add_library(XXX_Lib_Static STATIC ${ProjectSources})
set_target_properties(
	XXX_Lib_Static PROPERTIES
	OUTPUT_NAME "XXX"
	VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_compile_options(XXX_Lib_Static PRIVATE -Wall -Wextra -Werror -pedantic)
target_include_directories(XXX_Lib_Static PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries(XXX_Lib_Static stdc++fs spdlog libzmq sentry)

add_executable(XXX ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_compile_options(XXX PRIVATE -Wall -Wextra -Werror -pedantic)
target_include_directories(XXX PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries(XXX XXX_Lib_Shared)

install(TARGETS XXX XXX_Lib_Shared spdlog libzmq sentry
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
)

install(CODE [[
  file(GET_RUNTIME_DEPENDENCIES
    LIBRARIES $<TARGET_FILE:XXX_Lib_Shared>
    EXECUTABLES $<TARGET_FILE:XXX>
    RESOLVED_DEPENDENCIES_VAR _r_deps
    UNRESOLVED_DEPENDENCIES_VAR _u_deps
  )
  foreach(_file ${_r_deps})
    file(INSTALL
      DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
      TYPE SHARED_LIBRARY
      FOLLOW_SYMLINK_CHAIN
      FILES "${_file}"
    )
	message(STATUS "Detected dependency ${_file}")
  endforeach()
  list(LENGTH _u_deps _u_length)
  if("${_u_length}" GREATER 0)
    message(WARNING "Unresolved dependencies detected!")
  endif()
]])

# Update build number at every build
add_custom_command(
	TARGET XXX_Lib_Shared
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -P "${PROJECT_SOURCE_DIR}/cmake/AutoBuildNumber.cmake"
	COMMENT "${BoldBlue}Updating build number${ColourReset}"
)

# Update dist folder after release build
add_custom_command(
	TARGET XXX
	POST_BUILD
	COMMAND $<$<CONFIG:Release>:find>
	$<$<CONFIG:Release>:${PROJECT_SOURCE_DIR}/dist/temp/>
	$<$<CONFIG:Release>:-maxdepth>
	$<$<CONFIG:Release>:1>
	$<$<CONFIG:Release>:-type>
	$<$<CONFIG:Release>:f>
	$<$<CONFIG:Release>:-delete>
	COMMAND $<$<CONFIG:Release>:sh>
	$<$<CONFIG:Release>:${PROJECT_SOURCE_DIR}/scripts/ldd-copy-dependencies.sh>
	$<$<CONFIG:Release>:-b>
	$<$<CONFIG:Release>:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/XXX>
	$<$<CONFIG:Release>:-t>
	$<$<CONFIG:Release>:${PROJECT_SOURCE_DIR}/dist/temp>
	COMMAND $<$<CONFIG:Release>:sh>
	$<$<CONFIG:Release>:${PROJECT_SOURCE_DIR}/scripts/makeself/makeself.sh>
	$<$<CONFIG:Release>:--notemp>
	$<$<CONFIG:Release>:--sha256>
	$<$<CONFIG:Release>:${PROJECT_SOURCE_DIR}/dist/temp>
	$<$<CONFIG:Release>:"${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run">
	$<$<CONFIG:Release>:"${PROJECT_NAME}">
	COMMAND $<$<CONFIG:Release>:mv>
	$<$<CONFIG:Release>:-f>
	$<$<CONFIG:Release>:"${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run">
	$<$<CONFIG:Release>:${PROJECT_SOURCE_DIR}/dist/makeself/>
	COMMENT "${BoldBlue}Updating dist folder${ColourReset}"
)

# CPack
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
set(CPACK_GENERATOR "RPM" "DEB")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_RELEASE ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RPM_SPEC_MORE_DEFINE "%define _build_id_links none")
set(CPACK_OUTPUT_FILE_PREFIX ${PROJECT_SOURCE_DIR}/dist/packages)
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${PROJECT_SOURCE_DIR}/README.md)
set(CPACK_PACKAGE_CONTACT "Humanity")
include(InstallRequiredSystemLibraries)
include(CPack)

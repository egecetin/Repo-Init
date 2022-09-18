# Update dist folder and prepare makeself archive
include(Colorize)
include(CMakeCacheForScript)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")

	message(STATUS "${BoldBlue}Updating dist folder and preparing packages${ColourReset}")
	# Clear old files
	execute_process(COMMAND "find" "${PROJECT_SOURCE_DIR}/dist/temp/" "-maxdepth" "1" "-type" "f" "-delete")

	# Copy shared libraries
	execute_process(COMMAND "sh"
							"${PROJECT_SOURCE_DIR}/scripts/ldd-copy-dependencies.sh"
							"-b"
							"${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/${PROJECT_NAME}"
							"-t"
							"${PROJECT_SOURCE_DIR}/dist/temp"
							)

	# Copy crashpad executable
	execute_process(COMMAND "cp" "-f" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/crashpad_handler" "${PROJECT_SOURCE_DIR}/dist/temp")

	# Create makeself package
	execute_process(COMMAND "sh"
							"${PROJECT_SOURCE_DIR}/scripts/makeself/makeself.sh"
							"--notemp"
							"--sha256"
							"${PROJECT_SOURCE_DIR}/dist/temp"
							"${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run"
							"${PROJECT_NAME}"
							)

	# Move to another directory
	execute_process(COMMAND "mv"
							"-f"
							"${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run"
							"${PROJECT_SOURCE_DIR}/dist/makeself/"
							)

	message(STATUS "${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run generated")
endif()

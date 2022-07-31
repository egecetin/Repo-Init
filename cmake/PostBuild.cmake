# Update dist folder and prepare makeself archive
include(Colorize)
include(CMakeCacheForScript)

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

# Move debug symbols
execute_process(COMMAND "mv"
                        "-f"
                        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/${PROJECT_NAME}.dbg"
                        "${PROJECT_SOURCE_DIR}/dist/debug/${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.dbg"
                        )
execute_process(COMMAND "mv"
                        "-f"
                        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE}/lib${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}.dbg"
                        "${PROJECT_SOURCE_DIR}/dist/debug/lib${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.dbg"
                        )

message(STATUS "${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run generated")

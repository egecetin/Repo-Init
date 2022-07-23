# Update dist folder and prepare makeself archive
include(Colorize)
include(CMakeCacheForScript)

message(STATUS "${BoldBlue}Updating dist folder and preparing packages${ColourReset}")
execute_process(COMMAND "find" "${PROJECT_SOURCE_DIR}/dist/temp/" "-maxdepth" "1" "-type" "f" "-delete")
execute_process(COMMAND "sh"
                        "${PROJECT_SOURCE_DIR}/scripts/ldd-copy-dependencies.sh"
                        "-b"
                        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/XXX"
                        "-t"
                        "${PROJECT_SOURCE_DIR}/dist/temp"
                        )
execute_process(COMMAND "sh"
                        "${PROJECT_SOURCE_DIR}/scripts/makeself/makeself.sh"
                        "--notemp"
                        "--sha256"
                        "${PROJECT_SOURCE_DIR}/dist/temp"
                        "${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run"
                        "${PROJECT_NAME}"
                        )
execute_process(COMMAND "mv"
                        "-f"
                        "${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run"
                        "${PROJECT_SOURCE_DIR}/dist/makeself/"
                        )
message(STATUS "${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run generated")

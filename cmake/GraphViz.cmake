find_program(DOT_EXE "dot")

if(DOT_EXE)
    message(STATUS "dot found: ${DOT_EXE}")
else()
    message(STATUS "${BoldYellow}dot not found!${ColourReset}")
endif()

set(DOT_OUTPUT_TYPE "svg" CACHE STRING "Build a dependency graph. Options are dot output types: ps, png, pdf...")

if(DOT_EXE)
    add_custom_target(dependency-graph
        COMMAND ${CMAKE_COMMAND} ${PROJECT_SOURCE_DIR} --graphviz=${PROJECT_BINARY_DIR}/graphviz/${PROJECT_NAME}.dot
        COMMAND ${DOT_EXE} -T${DOT_OUTPUT_TYPE} ${PROJECT_BINARY_DIR}/graphviz/${PROJECT_NAME}.dot -o ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-tree.${DOT_OUTPUT_TYPE}
    )

    add_custom_command(
        TARGET dependency-graph POST_BUILD
        COMMAND ;
        COMMENT
        "Dependency graph generated and located at ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.${DOT_OUTPUT_TYPE}"
    )
endif()

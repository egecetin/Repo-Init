#create a pretty commit id using git
#uses 'git describe --tags', so tags are required in the repo
#create a tag with 'git tag <name>' and 'git push --tags'

find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags RESULT_VARIABLE res_var OUTPUT_VARIABLE GIT_COM_ID OUTPUT_QUIET ERROR_QUIET)
    if( NOT ${res_var} EQUAL 0 )
        message( WARNING " Git failed (not a repo, or no tags). Build will not contain git revision info." )
    else()
        string( REPLACE "\n" "" GIT_COMMIT_ID ${GIT_COM_ID} )
        message("Current revision is ${GIT_COMMIT_ID}")

        set(REVISION_VERSION ${GIT_COMMIT_ID})

        # file(WRITE version_string.hpp.txt "#define REV_VERSION \"${GIT_COMMIT_ID}\"\;\n" )
        # copy the file to the final header only if the version changes
        # reduces needless rebuilds
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                version_string.hpp.txt ${CMAKE_CURRENT_BINARY_DIR}/Version.hpp)
    endif()    
else()
    message( WARNING "Git not found. Build will not contain git revision info." )
endif()

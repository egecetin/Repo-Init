#create a pretty commit id using git
#uses 'git describe --tags', so tags are required in the repo
#create a tag with 'git tag <name>' and 'git push --tags'

find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD RESULT_VARIABLE res_var OUTPUT_VARIABLE GIT_COM_ID ERROR_QUIET)
    if( NOT ${res_var} EQUAL 0 )
        message(WARNING " Git failed (not a repo). Build will not contain git revision info.")
    else()
        message("Current revision is (from Git) ${GIT_COM_ID}")

        if(DEFINED ${REVISION_VERSION})
            message(WARNING " Revision version already set overwriting")
        endif()
        set(REVISION_VERSION ${GIT_COM_ID})
    endif()    
else()
    message(WARNING "Git not found. Build will not contain git revision info.")
endif()

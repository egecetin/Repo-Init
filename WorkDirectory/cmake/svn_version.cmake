
# the FindSubversion.cmake module is part of the standard distribution
include(CustomFindSubversion)
if(SUBVERSION_FOUND)
    # extract working copy information for SOURCE_DIR into MY_XXX variables
    Subversion_WC_INFO(${SOURCE_DIR} MY IGNORE_SVN_FAILURE)
    if(${MY_WC_REVISION})
        message(INFO " Current revision is ${MY_WC_REVISION}")

        set(REVISION_VERSION ${MY_WC_REVISION})

        # write a file with the SVNVERSION define
        # file(WRITE svnversion.h.txt "#define REV_VERSION ${MY_WC_REVISION}\n")
        # copy the file to the final header only if the version changes
        # reduces needless rebuilds
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                svnversion.h.txt ${CMAKE_CURRENT_BINARY_DIR}/Version.hpp)
    else()
        message(WARNING " SVN failed (not a repo). Build will not contain SVN revision info.")
    endif()
else()
    message( WARNING "SVN not found. Build will not contain SVN revision info.")
endif()
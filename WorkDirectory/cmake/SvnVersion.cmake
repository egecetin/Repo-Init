
# the FindSubversion.cmake module is part of the standard distribution
include(CustomFindSubversion)
if(SUBVERSION_FOUND)
    # extract working copy information for SOURCE_DIR into MY_XXX variables
    Subversion_WC_INFO(${SOURCE_DIR} MY IGNORE_SVN_FAILURE)
    if(${MY_WC_REVISION})
        message(INFO " Current revision is (from SVN) ${MY_WC_REVISION}")

        if(DEFINED ${REVISION_VERSION})
            message(WARNING " Revision version already set overwriting")
        endif()
        set(REVISION_VERSION ${MY_WC_REVISION})
    else()
        message(WARNING " SVN failed (not a repo). Build will not contain SVN revision info.")
    endif()
else()
    message( WARNING "SVN not found. Build will not contain SVN revision info.")
endif()
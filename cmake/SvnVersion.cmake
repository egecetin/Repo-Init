# From https://stackoverflow.com/questions/3780667/use-cmake-to-get-build-time-subversion-revision

include(CustomFindSubversion)
if(SUBVERSION_FOUND)
	# extract working copy information for SOURCE_DIR into MY_X variables
	Subversion_WC_INFO(${PROJECT_SOURCE_DIR} MY IGNORE_SVN_FAILURE)
	if(${MY_WC_REVISION})
		message("${BoldBlue}Current revision is (from SVN) ${MY_WC_REVISION}${ColourReset}")

		if(DEFINED ${REVISION_VERSION})
			message(WARNING " Revision version already set overwriting")
		endif()
		set(REVISION_VERSION ${MY_WC_REVISION})
	else()
		message(AUTHOR_WARNING " SVN failed (not a repo). Build will not contain SVN revision info.")
	endif()
else()
	message(AUTHOR_WARNING " SVN not found. Build will not contain SVN revision info.")
endif()

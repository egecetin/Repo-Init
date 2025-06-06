# From https://stackoverflow.com/questions/60955881/cmake-for-doxygen

find_package(Doxygen)
if(DOXYGEN_FOUND)
  # set input and output files
  set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in)
  set(DOXYGEN_OUT ${PROJECT_SOURCE_DIR}/Doxyfile)

  # request to configure the file
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

  # note the option ALL which allows to build the docs together with the application
  add_custom_target(
    docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
else()
  message(WARNING "Doxygen need to be installed to generate the doxygen documentation!")
endif()

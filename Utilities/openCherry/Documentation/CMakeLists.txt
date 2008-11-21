# 
# Variables:
#  OPENCHERRY_DOXYGEN_OUTPUT_DIR: doxygen output directory (optional)


OPTION(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
MARK_AS_ADVANCED(USE_DOT)

SET(HAVE_DOT "NO")
IF(DOT)
  IF(USE_DOT)
    SET(HAVE_DOT "YES")
  ENDIF(USE_DOT)
ENDIF(DOT)


SET(OPENCHERRY_DOXYGEN_OUTPUT_DIR ${OPENCHERRY_BINARY_DIR}/Documentation/Doxygen CACHE PATH "Output directory for doxygen generated documentation." )

CONFIGURE_FILE(doxygen.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)

FIND_PACKAGE(Doxygen)

# Doxygen config file requires only the path to DOT, not DOT itself
GET_FILENAME_COMPONENT(DOT_PATH ${DOT} PATH CACHE)

ADD_CUSTOM_TARGET(openCherryDoc 
                  ${DOXYGEN} 
                  ${OPENCHERRY_BINARY_DIR}/Documentation/doxygen.conf
                  )

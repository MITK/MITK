OPTION(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
MARK_AS_ADVANCED(USE_DOT)

SET(HAVE_DOT "NO")
IF(DOT)
  IF(USE_DOT)
    SET(HAVE_DOT "YES")
  ENDIF(USE_DOT)
ENDIF(DOT)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/Documentation/doxygen.conf.in
               ${PROJECT_BINARY_DIR}/Documentation/doxygen.conf)

FIND_PACKAGE(Doxygen)

# Doxygen config file requires only the path to DOT, not DOT itself
GET_FILENAME_COMPONENT(DOT_PATH ${DOT} PATH CACHE)

ADD_CUSTOM_TARGET(doc 
                  ${DOXYGEN} 
                  ${PROJECT_BINARY_DIR}/Documentation/doxygen.conf
                  )

# make sure the main app is built already so we get documentation for 
# the Qt generated files 
IF(USE_QT)
ADD_DEPENDENCIES(doc MainApp)
ENDIF(USE_QT)


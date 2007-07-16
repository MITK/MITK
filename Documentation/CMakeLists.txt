# 
# Variables:
#  MITK_DOXYGEN_OUTPUT_DIR: doxygen output directory (optional)


OPTION(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
MARK_AS_ADVANCED(USE_DOT)

SET(HAVE_DOT "NO")
IF(DOT)
  IF(USE_DOT)
    SET(HAVE_DOT "YES")
  ENDIF(USE_DOT)
ENDIF(DOT)

FOREACH(FUNC_DIR ${ALL_FUNCTIONALITY_DIRS})
  SET(USERMANUAL_DIR ${FUNC_DIR}/UserManual)
  IF(EXISTS ${USERMANUAL_DIR})
    SET(FUNCTIONALITY_USER_MANUALS "${FUNCTIONALITY_USER_MANUALS} ${USERMANUAL_DIR}" )
  ENDIF(EXISTS ${USERMANUAL_DIR})
  SET(FUNCTIONALITY_DIRS "${FUNCTIONALITY_DIRS} ${FUNC_DIR}")
ENDFOREACH(FUNC_DIR)

SET(MITK_DOXYGEN_OUTPUT_DIR ${MITK_BINARY_DIR}/Documentation/Doxygen CACHE PATH "Output directory for doxygen generated documentation." )

CONFIGURE_FILE(doxygen.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)

# CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/Documentation/doxygen.conf.in
#                ${PROJECT_BINARY_DIR}/Documentation/doxygen.conf)
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


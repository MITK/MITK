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

# this is copied from MainApp's CMakeLists.txt and should be refactored out in a MACRO
GET_DIRECTORY_PROPERTY(CACHE_VARS CACHE_VARIABLES)

FOREACH(kit MITK MBIQM MBISB)
  SET(${kit}_FUNCTIONALITY_NAMES_KNOWN "")

FOREACH(entry ${CACHE_VARS})
STRING(REGEX MATCH "^${kit}_KNOWN_FUNCTIONALITY_[A-Za-z0-9]+$" match ${entry})
IF(match)
  STRING(REGEX REPLACE "^${kit}_KNOWN_FUNCTIONALITY_" "" potential_func_name ${match})
  # now check whether a functionality dir is set and exists
  IF(${match})
    IF(IS_DIRECTORY ${${match}}/UserManual)
      IF(BUILD_${potential_func_name} OR BUILD_ALL_FUNCTIONALITIES)
      SET(USERMANUAL_DIRS "${USERMANUAL_DIRS} ${${match}}/UserManual")
      ENDIF(BUILD_${potential_func_name} OR BUILD_ALL_FUNCTIONALITIES)
    ENDIF(IS_DIRECTORY ${${match}}/UserManual)
  ENDIF(${match})
ENDIF(match)
ENDFOREACH(entry)
ENDFOREACH(kit)
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
IF(DESIRED_QT_VERSION EQUAL 3)
ADD_DEPENDENCIES(doc MainApp)
ENDIF(DESIRED_QT_VERSION EQUAL 3)


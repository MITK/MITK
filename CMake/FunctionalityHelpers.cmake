MACRO( FILL_WITH_BASENAMES DESTLIST SOURCELIST) 
  FOREACH(file ${${SOURCELIST}})
    GET_FILENAME_COMPONENT(BASENAME ${file} NAME)
    SET(${DESTLIST} ${${DESTLIST}} ${BASENAME})
  ENDFOREACH(file)
ENDMACRO( FILL_WITH_BASENAMES ) 

# legacy macro, called by "old" functionalities
MACRO( createFunctionalityLib )
  IF(NOT FUNC_NAME)
    GET_FILENAME_COMPONENT(FUNC_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    MESSAGE(STATUS "Warning. No FUNC_NAME. Setting it to directory name: ${FUNC_NAME}")
  ENDIF(NOT FUNC_NAME)
  MESSAGE(STATUS "Including ${FUNC_NAME} with legacy mechanisn. Fix it.")
  # remove all prepending paths from the files
  FILL_WITH_BASENAMES(FUNC_CPP APPMOD_CPP)
  FILL_WITH_BASENAMES(FUNC_MOC_H APPMOD_MOC_H)
  FILL_WITH_BASENAMES(H_LIST APPMOD_H)
  FILL_WITH_BASENAMES(FUNC_FORMS APPMOD_FORMS)
  CREATE_QFUNCTIONALITY(${FUNC_NAME})
ENDMACRO( createFunctionalityLib )

# 
# Variables
#
#   FUNC_NAME: Name of the functionality. If not set it will be set to the directory name
# 
# The following ones will be generated if not set
#   
#   ADD_FUNC_CODE: Name of a file which contains the AddFuncName() definition
#   ADD_FUNC_HEADER: Name of a file which contains the AddFuncName() declaration
#   ADD_FUNC_CALL: File which contains just the call to the above function 

MACRO(CREATE_QFUNCTIONALITY FUNC_NAME)
  IF(NOT FUNC_NAME)
    GET_FILENAME_COMPONENT(FUNC_NAME ${PROJECT_SOURCE_DIR} NAME)
    MESSAGE(STATUS "Warning. No FUNC_NAME set. Setting it to directory name: ${FUNC_NAME}")
  ENDIF(NOT FUNC_NAME)
   SET(FUNC_CPP_LIST ${FUNC_CPP})
  IF(FUNC_FORMS)
    QT_WRAP_UI(${FUNC_NAME} FUNC_H_LIST FUNC_CPP_LIST ${FUNC_FORMS})
  ENDIF(FUNC_FORMS)
  IF(FUNC_MOC_H)
    QT_WRAP_CPP(${FUNC_NAME} FUNC_CPP_LIST ${FUNC_MOC_H})
  ENDIF(FUNC_MOC_H)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  SET(FUNCTIONALITY_NAME ${FUNC_NAME})
  CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/Qmitk/QmitkApplicationBase/RegisterQFunctionality.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/Register${FUNC_NAME}.cpp" @ONLY)
  REMOVE(FUNCTIONALITY_NAMES ${FUNC_NAME})
  SET(FUNCTIONALITY_NAMES ${FUNCTIONALITY_NAMES};${FUNC_NAME} CACHE INTERNAL "Names of all enabled functionalities.")
  ADD_LIBRARY(${FUNC_NAME} ${FUNC_CPP_LIST} "${CMAKE_CURRENT_BINARY_DIR}/Register${FUNC_NAME}.cpp" )
  TARGET_LINK_LIBRARIES(${FUNC_NAME} Qmitk)
ENDMACRO(CREATE_QFUNCTIONALITY)



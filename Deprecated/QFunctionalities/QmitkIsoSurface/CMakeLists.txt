# legacy file to support the new functionality
# build mechanism with the old "functionality name".cmake style files

INCLUDE_DIRECTORIES(${QMITKFUNCTIONALITIES_INCLUDE_DIRS})
SET(APPMOD_CPP "")
SET(APPMOD_FORMS "")
SET(APPMOD_MOC_H "")
SET(APPMOD_H "")
GET_FILENAME_COMPONENT(TMP_FUNC_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
INCLUDE(${TMP_FUNC_NAME}.cmake)
SET(FUNC_NAME ${TMP_FUNC_NAME})
createFunctionalityLib()

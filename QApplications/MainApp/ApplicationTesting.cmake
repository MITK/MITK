# tests without data
IF(MITK_APPLICATION_TESTING)
  ADD_TEST(MainAppWithoutData ${EXECUTABLE_OUTPUT_PATH}/MainApp -noMITKOptions -testing)
  ADD_TEST(MainApp_EmptyImage ${EXECUTABLE_OUTPUT_PATH}/MainApp -testEmptyImage -noMITKOptions -testing)
  ADD_TEST(MainApp_EmptySurface ${EXECUTABLE_OUTPUT_PATH}/MainApp -testEmptySurface -noMITKOptions -testing)
  ADD_TEST(MainApp_EmptyPointSet ${EXECUTABLE_OUTPUT_PATH}/MainApp -testEmptyPointSet -noMITKOptions -testing)
  ADD_TEST(MainApp_EmptyNode ${EXECUTABLE_OUTPUT_PATH}/MainApp -testEmptyNode -noMITKOptions -testing)
ENDIF(MITK_APPLICATION_TESTING)
IF(MITK_FUNCTIONALITY_TESTING)
  FOREACH(FUNC_NAME ${FUNC_LIBRARIES})
    ADD_TEST(${FUNC_NAME}_WithoutData ${EXECUTABLE_OUTPUT_PATH}/MainApp -noMITKOptions -testing ${FUNC_NAME})
    ADD_TEST(${FUNC_NAME}_EmptyImage ${EXECUTABLE_OUTPUT_PATH}/MainApp -testEmptyImage -noMITKOptions -testing ${FUNC_NAME})
    ADD_TEST(${FUNC_NAME}_EmptySurface ${EXECUTABLE_OUTPUT_PATH}/MainApp -testEmptySurface -noMITKOptions -testing ${FUNC_NAME})
    ADD_TEST(${FUNC_NAME}_EmptyPointSet ${EXECUTABLE_OUTPUT_PATH}/MainApp -testEmptyPointSet -noMITKOptions -testing ${FUNC_NAME})
    ADD_TEST(${FUNC_NAME}_EmptyNode ${EXECUTABLE_OUTPUT_PATH}/MainApp -testEmptyNode -noMITKOptions -testing ${FUNC_NAME})
  ENDFOREACH(FUNC_NAME)
ENDIF(MITK_FUNCTIONALITY_TESTING)

# now test with all data files found
FILE(GLOB testdatalist ${MITK_SOURCE_DIR}/Testing/Data/*.*)
LIST(REMOVE_ITEM testdatalist "${MITK_SOURCE_DIR}/Testing/Data/.svn")
FOREACH(testdataitem ${testdatalist})

  GET_FILENAME_COMPONENT(testdataname ${testdataitem} NAME)

  IF(MITK_FUNCTIONALITY_TESTING)
    FOREACH(FUNC_NAME ${FUNC_LIBRARIES})
    ADD_TEST(${FUNC_NAME}_${testdataname} ${EXECUTABLE_OUTPUT_PATH}/MainApp ${testdataitem} -noMITKOptions -testing ${FUNC_NAME})
    ENDFOREACH(FUNC_NAME)
  ENDIF(MITK_FUNCTIONALITY_TESTING)

  IF(MITK_APPLICATION_TESTING)
    ADD_TEST(MainApp_${testdataname} ${EXECUTABLE_OUTPUT_PATH}/MainApp ${testdataitem} -noMITKOptions -testing)
  ENDIF(MITK_APPLICATION_TESTING)

ENDFOREACH(testdataitem)



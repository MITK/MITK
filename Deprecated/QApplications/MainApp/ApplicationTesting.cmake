# tests without data or empty nodes
SET(datatypes Image Surface PointSet Node) 
OPTION(MITK_APPLICATION_TESTING "Test the main application. Needs GUI" OFF)
OPTION(MITK_FUNCTIONALITY_TESTING "Test each functionlity on its own. Needs GUI" OFF)
IF(MITK_APPLICATION_TESTING)
  ADD_TEST(MainAppWithoutData ${EXECUTABLE_OUTPUT_PATH}/MainApp -noMITKOptions -testing)
  ADD_TEST(MainAppOptionTesting ${EXECUTABLE_OUTPUT_PATH}/MainApp -noMITKOptions -optiontesting)
  FOREACH(datatype ${datatypes})
    ADD_TEST(MainApp_Empty${datatype} ${EXECUTABLE_OUTPUT_PATH}/MainApp -testEmpty${datatype} -noMITKOptions -testing)
  ENDFOREACH(datatype)
ENDIF(MITK_APPLICATION_TESTING)
IF(MITK_FUNCTIONALITY_TESTING)
  FOREACH(FUNC_NAME ${FUNC_LIBRARIES})
    ADD_TEST(${FUNC_NAME}_WithoutData ${EXECUTABLE_OUTPUT_PATH}/MainApp -noMITKOptions -testing ${FUNC_NAME})
    FOREACH(datatype ${datatypes})
      ADD_TEST(${FUNC_NAME}_Empty${datatype} ${EXECUTABLE_OUTPUT_PATH}/MainApp -testEmpty${datatype} -noMITKOptions -testing ${FUNC_NAME})
    ENDFOREACH(datatype)
  ENDFOREACH(FUNC_NAME)
ENDIF(MITK_FUNCTIONALITY_TESTING)

# now test with some data files 
SET(testdatadir ${MITK_SOURCE_DIR}/Testing/Data/)
SET(testdatalist 
  binary.stl
  lungs.vtk
  Pic2DplusT.pic.gz
  Pic3D.pic.gz
  Png2D-bw.png
  US4DCyl.pic.gz
)
FOREACH(testdataname ${testdatalist})

  IF(MITK_FUNCTIONALITY_TESTING)
    FOREACH(FUNC_NAME ${FUNC_LIBRARIES})
    ADD_TEST(${FUNC_NAME}_${testdataname} ${EXECUTABLE_OUTPUT_PATH}/MainApp ${testdatadir}/${testdataname} -noMITKOptions -testing ${FUNC_NAME})
    ENDFOREACH(FUNC_NAME)
  ENDIF(MITK_FUNCTIONALITY_TESTING)

  IF(MITK_APPLICATION_TESTING)
    ADD_TEST(MainApp_${testdataname} ${EXECUTABLE_OUTPUT_PATH}/MainApp ${testdatadir}/${testdataname} -noMITKOptions -testing)
  ENDIF(MITK_APPLICATION_TESTING)

ENDFOREACH(testdataname)



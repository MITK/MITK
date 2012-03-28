
MITK_CREATE_MODULE_TESTS(LABELS MITK-Modules)

if(BUILD_TESTING AND MODULE_IS_ENABLED)
  add_test(mitkSceneIOTest_Pic3D.nrrd_binary.stl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkSceneIOTest ${MITK_DATA_DIR}/Pic3D.nrrd
 ${MITK_DATA_DIR}/binary.stl)
  set_property(TEST mitkSceneIOTest_Pic3D.nrrd_binary.stl PROPERTY LABELS MITK-Modules)
endif()

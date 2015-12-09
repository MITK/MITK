MITK_CREATE_MODULE_TESTS(LABELS MITK-Modules)

if(TARGET ${TESTDRIVER})
  if(BUILD_TESTING AND MODULE_IS_ENABLED)

    add_test(mitkSceneIOTest_Pic3D.nrrd_binary.stl
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER}
               mitkSceneIOTest
               ${MITK_DATA_DIR}/Pic3D.nrrd
               ${MITK_DATA_DIR}/binary.stl
            )
    set_property(TEST mitkSceneIOTest_Pic3D.nrrd_binary.stl PROPERTY LABELS MITK-Modules)

  if(MITK_ENABLE_RENDERING_TESTING)
    mitkAddCustomModuleTest(mitkSceneIOCompatibility_NoRainbowCT mitkSceneIOCompatibilityTest
                            ${MITK_DATA_DIR}/RenderingTestData/SceneFiles/rainbows-post-17547.mitk # scene to load
                            -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/rainbows-post-17547.png) # reference rendering

  if (false) # fix not yet integrated
    mitkAddCustomModuleTest(mitkSceneIOCompatibility_SurfaceIntLineWidth mitkSceneIOCompatibilityTest
                            ${MITK_DATA_DIR}/RenderingTestData/SceneFiles/surface-pre-18528.mitk # scene to load
                            -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/surface-pre-18528.png) # reference rendering
  endif()
  endif()

  endif()
endif()

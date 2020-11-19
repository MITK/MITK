MITK_CREATE_MODULE_TESTS()

if(TARGET ${TESTDRIVER})
  if(BUILD_TESTING AND MODULE_IS_ENABLED)

    add_test(mitkSceneIOTest_Pic3D.nrrd_binary.stl
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER}
               mitkSceneIOTest
               ${MITK_DATA_DIR}/Pic3D.nrrd
               ${MITK_DATA_DIR}/binary.stl
            )

    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_RELEASE release RELEASE)
    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_DEBUG debug DEBUG)
    set(test_env_path ${MITK_RUNTIME_PATH_RELEASE} ${MITK_RUNTIME_PATH_DEBUG} $ENV{PATH})
    list(REMOVE_DUPLICATES test_env_path)
    string (REGEX REPLACE "\;" "\\\;" test_env_path "${test_env_path}")
    set_property(TEST mitkSceneIOTest_Pic3D.nrrd_binary.stl APPEND PROPERTY ENVIRONMENT "PATH=${test_env_path}")
    set_property(TEST mitkSceneIOTest_Pic3D.nrrd_binary.stl PROPERTY SKIP_RETURN_CODE 77)

    mitkAddCustomModuleRenderingTest(mitkSceneIOCompatibility_NoRainbowCT mitkSceneIOCompatibilityTest
                            ${MITK_DATA_DIR}/RenderingTestData/SceneFiles/rainbows-post-17547.mitk # scene to load
                            -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/rainbows-post-17547.png) # reference rendering

    if (FALSE) # fix not yet integrated
      mitkAddCustomModuleRenderingTest(mitkSceneIOCompatibility_SurfaceIntLineWidth mitkSceneIOCompatibilityTest
                              ${MITK_DATA_DIR}/RenderingTestData/SceneFiles/surface-pre-18528.mitk # scene to load
                              -V ${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/surface-pre-18528.png) # reference rendering
    endif()

  endif()
endif()

MITK_CREATE_MODULE_TESTS()
if(BUILD_TESTING AND MODULE_IS_ENABLED)
  mitkAddCustomModuleTest(mitkAnisotropicIterativeClosestPointRegistrationTest_bunnyICP mitkAnisotropicIterativeClosestPointRegistrationTest
                        ${MITK_MBI_DATA_DIR}/AICPTestData/bunny_scaled_3000_reoriented_ICP.stl #fixed surface
                        ${MITK_MBI_DATA_DIR}/AICPTestData/bunny_scaled_1000_reoriented_ICP.stl #moving surface
                        ${MITK_MBI_DATA_DIR}/AICPTestData/bunny_target_point_3000_ICP.mps #targets fixed surface
                        ${MITK_MBI_DATA_DIR}/AICPTestData/bunny_target_point_1000_ICP.mps #targets moving surface
                        0.0 #trimmed part (0.0: do not use trimmed AICP)
                        3.15857 #FRE from literature: \e130-projekte\NeedleNavigation\AnisotropicICP\EvaluationTPAMI\Experimente_MÃ¤rz2011/Ergebnisse.xlsx
                        0.0784612 #TRE from literature: \e130-projekte\NeedleNavigation\AnisotropicICP\EvaluationTPAMI\Experimente_MÃ¤rz2011/Ergebnisse.xlsx
)

mitkAddCustomModuleTest(mitkAnisotropicIterativeClosestPointRegistrationTest_faceTrimmedAICP mitkAnisotropicIterativeClosestPointRegistrationTest
                        ${MITK_MBI_DATA_DIR}/AICPTestData/head_130deg.stl #fixed surface
                        ${MITK_MBI_DATA_DIR}/AICPTestData/head_120deg.stl #moving surface
                        ${MITK_MBI_DATA_DIR}/AICPTestData/head_targets_130deg.mps #targets fixed surface
                        ${MITK_MBI_DATA_DIR}/AICPTestData/head_targets_120deg.mps #targets moving surface
                        0.2 #trimmed part 0.2 = 80% in GUI
                        13.511 #FRE from literature: /E130-Projekte/ToF/Studies/AICP_SPIE/SPIE2012/results/AllResultsForTrimmed80.ods
                        3.49151 #TRE from literature: /E130-Projekte/ToF/Studies/AICP_SPIE/SPIE2012/results/AllResultsForTrimmed80.ods
)

#removed due to windows issues #11548
mitkAddCustomModuleTest(mitkCovarianceMatrixTest mitkCovarianceMatrixCalculatorTest
                        ${MITK_MBI_DATA_DIR}/AICPTestData/head_130deg.stl #fixed surface
)

endif() #end BUILD_TESTING AND MODULE_IS_ENABLED


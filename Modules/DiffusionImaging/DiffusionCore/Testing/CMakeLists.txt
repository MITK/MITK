# The DiffusionCore tests need relaxed compiler flags...
# TODO fix tests to compile without these additional no-error flags
if(MSVC_VERSION)
  # disable deprecated warnings (they would lead to errors)
  mitkFunctionCheckCAndCXXCompilerFlags("/wd4996" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
else()
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=deprecated" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=deprecated-declarations" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

MITK_CREATE_MODULE_TESTS()
if(TARGET ${TESTDRIVER})
  mitk_use_modules(TARGET ${TESTDRIVER} PACKAGES ITK|ITKIOTransformBase+ITKDiffusionTensorImage)

  mitkAddCustomModuleTest(mitkImageReconstructionTest mitkImageReconstructionTest ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test.dwi ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_dti.dti ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_QN0.qbi ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_QA0.qbi ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_QA6.qbi ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_QA4.qbi ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_QA5.qbi)
endif()

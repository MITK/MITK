MITK_CREATE_MODULE_TESTS()

mitkAddCustomModuleTest(mitkFiberBundleXReaderWriterTest mitkFiberBundleXReaderWriterTest ${MITK_DATA_DIR}/DiffusionImaging/fiberBundleX.fib)
# deactivated. waiting for new tarball
#mitkAddCustomModuleTest(mitkGibbsTrackingTest mitkGibbsTrackingTest ${MITK_DATA_DIR}/DiffusionImaging/qBallImage.qbi ${MITK_DATA_DIR}/DiffusionImaging/diffusionImageMask.nrrd ${MITK_DATA_DIR}/DiffusionImaging/gibbsTrackingParameters.gtp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ ${MITK_DATA_DIR}/DiffusionImaging/fiberBundleX.fib)
## deactivated, see bug 12017
# mitkAddCustomModuleTest(mitkFiberBundleXTest mitkFiberBundleXTest ${MITK_DATA_DIR}/DiffusionImaging/fiberBundleX.fib)


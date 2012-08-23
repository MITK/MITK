MITK_CREATE_MODULE_TESTS()


mitkAddCustomModuleTest(mitkFiberBundleXReaderWriterTest mitkFiberBundleXReaderWriterTest ${MITK_DATA_DIR}/DiffusionImaging/fiberBundleX.fib)
mitkAddCustomModuleTest(mitkGibbsTrackingTest mitkGibbsTrackingTest ${MITK_DATA_DIR}/DiffusionImaging/qBallImage.qbi ${MITK_DATA_DIR}/DiffusionImaging/diffusionImageMask.nrrd ${MITK_DATA_DIR}/DiffusionImaging/gibbsTrackingParameters.gtp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ ${MITK_DATA_DIR}/DiffusionImaging/fiberBundleX.fib)
#mitkAddCustomModuleTest(mitkFiberBundleXTest mitkFiberBundleXTest ${MITK_DATA_DIR}/DiffusionImaging/fiberBundleX.fib)


mitkAddCustomModuleTest(mitkTbssNrrdImageReaderTest mitkTbssNrrdImageReaderTest ${MITK_DATA_DIR}/DiffusionImaging/tbss.tbss)
#mitkAddCustomModuleTest(mitkTbssRoiNrrdImageReaderTest mitkTbssRoiNrrdImageReaderTest ${MITK_DATA_DIR}/DiffusionImaging/bodyfornix.roi)
mitkAddCustomModuleTest(mitkTbssNrrdImageWriterTest mitkTbssNrrdImageWriterTest ${MITK_DATA_DIR}/DiffusionImaging/tbss.tbss ${MITK_DATA_DIR}/DiffusionImaging/tbss2.tbss)


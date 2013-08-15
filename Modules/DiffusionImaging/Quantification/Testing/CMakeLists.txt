MITK_CREATE_MODULE_TESTS()

mitkAddCustomModuleTest(mitkTbssNrrdImageReaderTest mitkTbssNrrdImageReaderTest ${MITK_DATA_DIR}/DiffusionImaging/tbss.tbss)
#mitkAddCustomModuleTest(mitkTbssRoiNrrdImageReaderTest mitkTbssRoiNrrdImageReaderTest ${MITK_DATA_DIR}/DiffusionImaging/bodyfornix.roi)
mitkAddCustomModuleTest(mitkTbssNrrdImageWriterTest mitkTbssNrrdImageWriterTest ${MITK_DATA_DIR}/DiffusionImaging/tbss.tbss ${MITK_DATA_DIR}/DiffusionImaging/tbss2.tbss)
mitkAddCustomModuleTest(mitkTbssSkeletonizationTest mitkTbssSkeletonizationTest ${MITK_DATA_DIR}/DiffusionImaging/mean_FA.nii.gz ${MITK_DATA_DIR}/DiffusionImaging/mean_FA_skeleton.nii.gz)

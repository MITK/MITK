MITK_CREATE_MODULE_TESTS(EXTRA_DRIVER_INIT "RegisterCoreExtObjectFactory();;"
                         EXTRA_DRIVER_INCLUDE "mitkCoreExtObjectFactory.h")

mitkAddCustomModuleTest(mitkLabeledImageToSurfaceFilterTest_BinaryBall  mitkLabeledImageToSurfaceFilterTest ${MITK_DATA_DIR}/BallBinary30x30x30.pic.gz)
mitkAddCustomModuleTest(mitkImageToSurfaceFilterTest_BinaryBall  mitkImageToSurfaceFilterTest ${MITK_DATA_DIR}/BallBinary30x30x30.nrrd)


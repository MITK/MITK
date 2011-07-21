MITK_CREATE_MODULE_TESTS(EXTRA_DRIVER_INIT "RegisterCoreExtObjectFactory();;"
                         EXTRA_DRIVER_INCLUDE "mitkCoreExtObjectFactory.h")

if(BUILD_TESTING AND MODULE_IS_ENABLED)
mitkAddCustomModuleTest(mitkLabeledImageToSurfaceFilterTest_BinaryBall  mitkLabeledImageToSurfaceFilterTest ${MITK_DATA_DIR}/BallBinary30x30x30.pic.gz)
endif()


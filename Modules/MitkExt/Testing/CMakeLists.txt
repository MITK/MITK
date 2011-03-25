MITK_CREATE_MODULE_TESTS(EXTRA_DRIVER_INIT "RegisterCoreExtObjectFactory();;"
                         EXTRA_DRIVER_INCLUDE "mitkCoreExtObjectFactory.h")

mitkAddCustomModuleTest(mitkLabeledImageToSurfaceFilterTest  mitkLabeledImageToSurfaceFilterTest ${MITK_SOURCE_DIR}/Core/Code/Testing/Data/BallBinary30x30x30.pic.gz)




MITK_CREATE_MODULE_TESTS()

ADD_TEST(mitkLabeledImageToSurfaceFilterTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkLabeledImageToSurfaceFilterTest ${MITK_SOURCE_DIR}/Core/Testing/Data/BallBinary30x30x30.pic.gz)



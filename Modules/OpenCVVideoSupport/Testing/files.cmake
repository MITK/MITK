set(MODULE_TESTS
)

set(MODULE_IMAGE_TESTS
  mitkImageToOpenCVImageFilterTest.cpp
  mitkConvertGrayscaleOpenCVImageFilterTest.cpp
  mitkCropOpenCVImageFilterTest.cpp
)

# list of images for which the tests are run
set(MODULE_TESTIMAGES
  Png2D-bw.png
  RenderingTestData/rgbImage.png
#NrrdWritingTestImage.jpg
)

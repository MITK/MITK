/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkImage.h"
#include "mitkImageGenerator.h"
#include "mitkTestingMacros.h"
#include "mitkImageSliceSelector.h"
#include "mitkTestFixture.h"

class mitkImageEqualTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkImageEqualTestSuite);
  MITK_TEST(Equal_CloneAndOriginal_ReturnsTrue);
  MITK_TEST(Equal_DifferentImageGeometry_ReturnsFalse);
  MITK_TEST(Equal_DifferentPixelTypes_ReturnsFalse);
  MITK_TEST(Equal_DifferentDimensions_ReturnsFalse);
  MITK_TEST(Equal_DifferentDimensionalities_ReturnsFalse);
  MITK_TEST(Equal_DifferentPixelValues_ReturnsFalse);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
  mitk::Image::Pointer m_Image;
  mitk::Image::Pointer m_AnotherImage;

public:

  /**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
  void setUp() override
  {
    //generate a gradient test image
    m_Image = mitk::ImageGenerator::GenerateGradientImage<unsigned char>(3u, 3u, 1u);
    m_AnotherImage = m_Image->Clone();
  }

  void tearDown() override
  {
    m_Image = nullptr;
    m_AnotherImage = nullptr;
  }

  void Equal_CloneAndOriginal_ReturnsTrue()
  {
    MITK_ASSERT_EQUAL( m_Image, m_Image->Clone(), "A clone should be equal to its original.");
  }

  void Equal_DifferentImageGeometry_ReturnsFalse()
  {
    mitk::Point3D origin;
    origin[0] = 0.0;
    origin[1] = 0.0;
    origin[2] = mitk::eps * 1.01;

    m_AnotherImage->GetGeometry()->SetOrigin(origin);

    MITK_ASSERT_NOT_EQUAL( m_Image, m_AnotherImage, "One origin was modified. Result should be false.");
  }

  void Equal_DifferentPixelTypes_ReturnsFalse()
  {
    m_AnotherImage = mitk::ImageGenerator::GenerateGradientImage<float>(3u, 3u, 1u);

    MITK_ASSERT_NOT_EQUAL( m_Image, m_AnotherImage, "One pixel type is float, the other unsigned char. Result should be false.");
  }

  void Equal_DifferentDimensions_ReturnsFalse()
  {
    m_AnotherImage = mitk::ImageGenerator::GenerateGradientImage<unsigned char>(5u, 7u, 3u);

    MITK_ASSERT_NOT_EQUAL( m_Image, m_AnotherImage, "Dimensions of first image are: (3, 3, 1). Dimensions of second image are: (5, 7, 3). Result should be false.");
  }

  void Equal_DifferentDimensionalities_ReturnsFalse()
  {
    //Select the first slice of a 2D image and compare it to the 3D original
    mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();
    sliceSelector->SetInput( m_Image );
    sliceSelector->SetSliceNr( 0 );
    sliceSelector->Update();
    m_AnotherImage = sliceSelector->GetOutput();

    MITK_ASSERT_NOT_EQUAL( m_Image, m_AnotherImage, "First image is 3D. Second image is 2D. Result should be false.");
  }

  void Equal_DifferentPixelValues_ReturnsFalse()
  {
    //todo: Replace the random images via simpler images with fixed values.
    m_Image = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(3u, 3u);
    m_AnotherImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(3u, 3u);

    MITK_ASSERT_NOT_EQUAL( m_Image, m_AnotherImage, "We compare two random images. Result should be false.");
  }

  void Equal_EpsilonDifference_ReturnsTrue()
  {
    m_Image = mitk::ImageGenerator::GenerateRandomImage<double>(10, 10);
    m_AnotherImage = m_Image->Clone();

    CPPUNIT_ASSERT_MESSAGE("Epsilon = 0.0 --> double images should not be regarded as equal", !mitk::Equal(*m_Image, *m_AnotherImage, 0, true));
    CPPUNIT_ASSERT_MESSAGE("Epsilon = 0.001 --> double images should be regarded as equal", mitk::Equal(*m_Image, *m_AnotherImage, 0.001, true));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageEqual)

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

/** Members used inside the different (sub-)tests. All members are initialized via Setup().*/
mitk::Image::Pointer m_Image;
mitk::Image::Pointer m_AnotherImage;

/**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
static void Setup()
{
  //generate a gradient test image
  m_Image = mitk::ImageGenerator::GenerateGradientImage<unsigned char>(3u, 3u, 1u);
  m_AnotherImage = m_Image->Clone();
}

void Equal_CloneAndOriginal_ReturnsTrue()
{
  Setup();
  MITK_TEST_EQUAL( m_Image, m_AnotherImage, "A clone should be equal to its original.");
}

static void Equal_InputIsNull_ReturnsFalse()
{
  mitk::Image::Pointer image = NULL;
  MITK_TEST_NOT_EQUAL( image, image, "Input is NULL. Result should be false.");
}

static void Equal_DifferentImageGeometry_ReturnsFalse()
{
  Setup();

  mitk::Point3D origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  origin[2] = mitk::eps * 1.01;

  m_AnotherImage->GetGeometry()->SetOrigin(origin);

  MITK_TEST_NOT_EQUAL( m_Image, m_AnotherImage, "One origin was modified. Result should be false.");
}

static void Equal_DifferentPixelTypes_ReturnsFalse()
{
  Setup();

  m_AnotherImage = mitk::ImageGenerator::GenerateGradientImage<float>(3u, 3u, 1u);

  MITK_TEST_NOT_EQUAL( m_Image, m_AnotherImage, "One pixel type is float, the other unsigned char. Result should be false.");
}

static void Equal_DifferentDimensions_ReturnsFalse()
{
  Setup();

  m_AnotherImage = mitk::ImageGenerator::GenerateGradientImage<unsigned char>(5u, 7u, 3u);

  MITK_TEST_NOT_EQUAL( m_Image, m_AnotherImage, "Dimensions of first image are: (3, 3, 1). Dimensions of second image are: (5, 7, 3). Result should be false.");
}

static void Equal_DifferentDimensionalities_ReturnsFalse()
{
  Setup();

  //Select the first slice of a 2D image and compare it to the 3D original
  mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();
  sliceSelector->SetInput( m_Image );
  sliceSelector->SetSliceNr( 0 );
  sliceSelector->Update();
  m_AnotherImage = sliceSelector->GetOutput();

  MITK_TEST_NOT_EQUAL( m_Image, m_AnotherImage, "First image is 3D. Second image is 2D. Result should be false.");
}

static void Equal_DifferentPixelValues_ReturnsFalse()
{
  //todo: Replace the random images via simpler images with fixed values.
  m_Image = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(3u, 3u);
  m_AnotherImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(3u, 3u);

  MITK_TEST_NOT_EQUAL( m_Image, m_AnotherImage, "We compare two random images. Result should be false.");
}

/**
 * @brief mitkImageAreEqualTest A test class for Equal methods in mitk::Image.
 */
int mitkImageEqualTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkImageAreEqualTest);

  Equal_CloneAndOriginal_ReturnsTrue();
  Equal_InputIsNull_ReturnsFalse();
  Equal_DifferentImageGeometry_ReturnsFalse();
  Equal_DifferentPixelTypes_ReturnsFalse();
  Equal_DifferentDimensions_ReturnsFalse();
  Equal_DifferentDimensionalities_ReturnsFalse();
  Equal_DifferentPixelValues_ReturnsFalse();

  MITK_TEST_END();
}

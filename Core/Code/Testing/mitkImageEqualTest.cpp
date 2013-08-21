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
#include <mitkImageSliceSelector.h>

mitk::Image::Pointer m_Image;
mitk::Image::Pointer m_AnotherImage;

/**
 * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the two used members for a new test case.
 */
static void Setup()
{
  //generate a gradient test image
  m_Image = mitk::ImageGenerator::GenerateGradientImage<unsigned char>(3u, 3u, 1u);
  m_AnotherImage = m_Image->Clone();
}

static void AreEqual_CloneAndOriginal_ReturnsTrue()
{
  Setup();
  MITK_TEST_CONDITION_REQUIRED( mitk::Equal( m_Image, m_AnotherImage), "A clone should be equal to its original.");
}

static void AreEqual_InputIsNull_ReturnsFalse()
{
  Setup();

  m_Image = NULL;
  m_AnotherImage = NULL;

  MITK_TEST_CONDITION_REQUIRED( !mitk::Equal( m_Image, m_AnotherImage), "Input is NULL. Result should be false.");
}

static void AreEqual_DifferentImageGeometry_ReturnsFalse()
{
  Setup();

  mitk::Point3D origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  origin[2] = mitk::eps * 1.01;

  m_AnotherImage->GetGeometry()->SetOrigin(origin);

  MITK_TEST_CONDITION_REQUIRED( !mitk::Equal( m_Image, m_AnotherImage), "One origin was modified. Result should be false.");
}

static void AreEqual_DifferentPixelTypes_ReturnsFalse()
{
  Setup();

  m_AnotherImage = mitk::ImageGenerator::GenerateGradientImage<float>(3u, 3u, 1u);

  MITK_TEST_CONDITION_REQUIRED( !mitk::Equal( m_Image, m_AnotherImage), "One pixel type is float, the other unsigned char. Result should be false.");
}

static void AreEqual_DifferentDimensions_ReturnsFalse()
{
  Setup();

  m_AnotherImage = mitk::ImageGenerator::GenerateGradientImage<unsigned char>(5u, 7u, 3u);

  MITK_TEST_CONDITION_REQUIRED( !mitk::Equal( m_Image, m_AnotherImage), "Dimensions of first image are: (3, 3, 1). Dimensions of second image are: (5, 7, 3). Result should be false.");
}

static void AreEqual_DifferentDimensionalities_ReturnsFalse()
{
  Setup();

  //Select the first slice of a 2D image and compare it to the 3D original
  mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();
  sliceSelector->SetInput( m_Image );
  sliceSelector->SetSliceNr( 0 );
  sliceSelector->Update();
  m_AnotherImage = sliceSelector->GetOutput();

  MITK_TEST_CONDITION_REQUIRED( !mitk::Equal( m_Image, m_AnotherImage), "First image is 3D. Second image is 2D. Result should be false.");
}

static void AreEqual_DifferentPixelValues_ReturnsFalse()
{
  m_Image = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(3u, 3u);
  m_AnotherImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(3u, 3u);

  MITK_TEST_CONDITION_REQUIRED( !mitk::Equal( m_Image, m_AnotherImage), "We compare  two random images. Result should be false.");
}

/**
 * @brief mitkImageAreEqualTest A test class for AreEqual methods in mitk::Image.
 */
int mitkImageEqualTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkImageAreEqualTest);

  AreEqual_CloneAndOriginal_ReturnsTrue();
  AreEqual_InputIsNull_ReturnsFalse();
  AreEqual_DifferentImageGeometry_ReturnsFalse();
  AreEqual_DifferentPixelTypes_ReturnsFalse();
  AreEqual_DifferentDimensions_ReturnsFalse();
  AreEqual_DifferentDimensionalities_ReturnsFalse();
  AreEqual_DifferentPixelValues_ReturnsFalse();

  MITK_TEST_END();

  return EXIT_SUCCESS;
}

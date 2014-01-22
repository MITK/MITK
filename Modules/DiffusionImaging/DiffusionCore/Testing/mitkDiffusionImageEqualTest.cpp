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

#include "mitkDiffusionImage.h"
#include "mitkIOUtil.h"
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

class mitkDiffusionImageEqualTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkDiffusionImageEqualTestSuite);
  MITK_TEST(Equal_CloneAndOriginal_ReturnsTrue);
  MITK_TEST(Equal_InputIsNull_ReturnsFalse);
//  MITK_TEST(Equal_DifferentGradientContainer_ReturnsFalse);
//  MITK_TEST(Equal_DifferentBValue_ReturnsFalse);
//  MITK_TEST(Equal_DifferentChannels_ReturnFalse);
//  MITK_TEST(Equal_MeasurmentFrame_ReturnFalse);

  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
  mitk::DiffusionImage<short>::Pointer m_Image;
  mitk::DiffusionImage<short>::Pointer m_AnotherImage;

public:

  /**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
  void setUp()
  {
    //generate a gradient test image
    std::string imagePath = GetTestDataFilePath("DiffusionImaging/ImageReconstruction/test.dwi");
    mitk::Image::Pointer image = mitk::IOUtil::LoadImage(imagePath);
    m_Image = static_cast<mitk::DiffusionImage<short>*>( image.GetPointer() );
    m_AnotherImage = m_Image->Clone();
  }

  void tearDown()
  {
    m_Image = NULL;
    m_AnotherImage = NULL;
  }

  void Equal_CloneAndOriginal_ReturnsTrue()
  {
    MITK_ASSERT_EQUAL( m_Image, m_Image->Clone(), "A clone should be equal to its original.");
  }

  void Equal_InputIsNull_ReturnsFalse()
  {
    mitk::Image::Pointer image = NULL;
    MITK_ASSERT_NOT_EQUAL( image, image, "Input is NULL. Result should be false.");
  }


};

MITK_TEST_SUITE_REGISTRATION(mitkDiffusionImageEqual)

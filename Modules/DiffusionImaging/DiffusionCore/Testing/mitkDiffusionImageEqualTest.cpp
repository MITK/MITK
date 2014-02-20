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
#include "itkDwiGradientLengthCorrectionFilter.h"

class mitkDiffusionImageEqualTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkDiffusionImageEqualTestSuite);
  MITK_TEST(Equal_CloneAndOriginal_ReturnsTrue);
  MITK_TEST(Equal_DifferentGradientContainerElements_ReturnsFalse);
  MITK_TEST(Equal_DifferentGradientContainerSize_ReturnsFalse);
  MITK_TEST(Equal_DifferentBValue_ReturnsFalse);
  MITK_TEST(Equal_DifferentVectorImagePixel_ReturnFalse);
  MITK_TEST(Equal_GetNumberOfBValues_ReturnFalse);
  MITK_TEST(Equal_DifferentMeasurmentFrame_ReturnFalse);
  //MITK_TEST(Equal_DifferentChannels_ReturnFalse);


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
    std::string imagePath = GetTestDataFilePath("DiffusionImaging/ImageReconstruction/test_multi.dwi");
    m_Image = static_cast<mitk::DiffusionImage<short>*>( mitk::IOUtil::LoadImage(imagePath).GetPointer());
    m_AnotherImage = m_Image->Clone();
  }

  void tearDown()
  {
    m_Image = NULL;
    m_AnotherImage = NULL;
  }

  void Equal_CloneAndOriginal_ReturnsTrue()
  {
    MITK_ASSERT_EQUAL( m_Image, m_AnotherImage, "A clone should be equal to its original.");
  }

  void Equal_DifferentGradientContainerElements_ReturnsFalse()
  {
    mitk::DiffusionImage<short>::GradientDirectionType dummyVec;
    dummyVec.fill(std::numeric_limits<double>::max());
    mitk::DiffusionImage<short>::GradientDirectionContainerType::ElementIdentifier mid_pos(m_AnotherImage->GetDirections()->Size()*0.5);
    m_AnotherImage->GetDirections()->SetElement(mid_pos, dummyVec);
    MITK_ASSERT_NOT_EQUAL( m_Image, m_AnotherImage, "GradientDirectionContainer Elements are not equal.");
  }

  void Equal_DifferentGradientContainerSize_ReturnsFalse()
  {
    mitk::DiffusionImage<short>::GradientDirectionContainerType::ElementIdentifier mid_pos(m_AnotherImage->GetDirections()->Size()*0.5);
    m_AnotherImage->GetDirections()->DeleteIndex(mid_pos);
    m_AnotherImage->GetDirections()->Squeeze();
    MITK_ASSERT_NOT_EQUAL( m_Image, m_AnotherImage, "GradientDirectionContainer Size is not equal.");
  }

  void Equal_DifferentBValue_ReturnsFalse()
  {
    m_AnotherImage->SetReferenceBValue(std::numeric_limits<unsigned int>::max());
    MITK_ASSERT_NOT_EQUAL( m_Image, m_AnotherImage, "BValue is not equal.");
  }

  void Equal_DifferentVectorImagePixel_ReturnFalse()
  {
    mitk::DiffusionImage<short>::ImageType::IndexType indx;
    indx.Fill(0);
    mitk::DiffusionImage<short>::ImageType::PixelType pix;
    short* val = new short[pix.Size()];
    pix.SetData(val);
    pix.Fill(std::numeric_limits<short>::min());
    m_AnotherImage->GetVectorImage()->SetPixel(indx,pix);
    MITK_ASSERT_NOT_EQUAL( m_Image, m_AnotherImage, "itkVectorImage voxel at pos [0,0,0] should be different.");
  }

  void Equal_DifferentMeasurmentFrame_ReturnFalse()
  {
    mitk::DiffusionImage<short>::MeasurementFrameType null_measurementframe;
    null_measurementframe.fill(0);
    m_AnotherImage->SetMeasurementFrame(null_measurementframe);
    MITK_ASSERT_NOT_EQUAL( m_Image, m_AnotherImage, "MeasurementFrame is an empty matrix. Result should be false.");
  }

  void Equal_GetNumberOfBValues_ReturnFalse()
  {
    itk::DwiGradientLengthCorrectionFilter::Pointer lengthCorrectionFilter = itk::DwiGradientLengthCorrectionFilter::New();
    lengthCorrectionFilter->SetReferenceBValue(m_AnotherImage->GetReferenceBValue());
    lengthCorrectionFilter->SetReferenceGradientDirectionContainer(m_AnotherImage->GetDirections());
    lengthCorrectionFilter->SetRoundingValue(10000);
    lengthCorrectionFilter->Update();
    m_AnotherImage->SetDirections(lengthCorrectionFilter->GetOutputGradientDirectionContainer());
    MITK_ASSERT_NOT_EQUAL( m_Image, m_AnotherImage, "NumberOfBValues should different.");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkDiffusionImageEqual)

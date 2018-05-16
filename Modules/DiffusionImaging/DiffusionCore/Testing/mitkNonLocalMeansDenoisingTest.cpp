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

#include "mitkIOUtil.h"
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"
#include "itkNonLocalMeansDenoisingFilter.h"
#include "mitkGradientDirectionsProperty.h"
#include "mitkITKImageImport.h"
#include <mitkImageCast.h>

class mitkNonLocalMeansDenoisingTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkNonLocalMeansDenoisingTestSuite);
  MITK_TEST(Denoise_NLMg_shouldReturnTrue);
  MITK_TEST(Denoise_NLMr_shouldReturnTrue);
  MITK_TEST(Denoise_NLMv_shouldReturnTrue);
  MITK_TEST(Denoise_NLMvr_shouldReturnTrue);
  CPPUNIT_TEST_SUITE_END();

private:

  typedef itk::VectorImage<short,3> VectorImagetType;

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
  mitk::Image::Pointer m_Image;
  mitk::Image::Pointer m_ReferenceImage;
  mitk::Image::Pointer m_DenoisedImage;
  itk::Image<short, 3>::Pointer m_ImageMask;
  itk::NonLocalMeansDenoisingFilter<short>::Pointer m_DenoisingFilter;

public:

  /**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
  void setUp() override
  {
    //generate test images
    std::string imagePath = GetTestDataFilePath("DiffusionImaging/Denoising/test_multi.dwi");

    m_Image =  mitk::IOUtil::Load<mitk::Image>(imagePath);
    m_ReferenceImage = nullptr;
    m_DenoisedImage = mitk::Image::New();

    //initialise Filter
    m_DenoisingFilter = itk::NonLocalMeansDenoisingFilter<short>::New();
    VectorImagetType::Pointer vectorImage;
    mitk::CastToItkImage(m_Image,vectorImage);
    m_DenoisingFilter->SetInputImage(vectorImage);
    m_DenoisingFilter->SetNumberOfThreads(1);
    m_DenoisingFilter->SetComparisonRadius(1);
    m_DenoisingFilter->SetSearchRadius(1);
    m_DenoisingFilter->SetVariance(500);
  }

  void tearDown() override
  {
    m_Image = nullptr;
    m_ImageMask = nullptr;
    m_ReferenceImage = nullptr;
    m_DenoisingFilter = nullptr;
    m_DenoisedImage = nullptr;
  }

  void Denoise_NLMg_shouldReturnTrue()
  {
    std::string referenceImagePath = GetTestDataFilePath("DiffusionImaging/Denoising/test_multi_NLMg.dwi");
    m_ReferenceImage =  mitk::IOUtil::Load<mitk::Image>(referenceImagePath);

    m_DenoisingFilter->SetUseRicianAdaption(false);
    m_DenoisingFilter->SetUseJointInformation(false);
    try
    {
      m_DenoisingFilter->Update();
    }
    catch(std::exception& e)
    {
      MITK_ERROR << e.what();
    }

    mitk::GrabItkImageMemory(m_DenoisingFilter->GetOutput(),m_DenoisedImage);
    m_DenoisedImage->SetPropertyList(m_Image->GetPropertyList()->Clone());

    MITK_ASSERT_EQUAL( m_DenoisedImage, m_ReferenceImage, "NLMg should always return the same result.");
  }

  void Denoise_NLMr_shouldReturnTrue()
  {
    std::string referenceImagePath = GetTestDataFilePath("DiffusionImaging/Denoising/test_multi_NLMr.dwi");
    m_ReferenceImage = mitk::IOUtil::Load<mitk::Image>(referenceImagePath);

    m_DenoisingFilter->SetUseRicianAdaption(true);
    m_DenoisingFilter->SetUseJointInformation(false);
    try
    {
      m_DenoisingFilter->Update();
    }
    catch(std::exception& e)
    {
      MITK_ERROR << e.what();
    }

    mitk::GrabItkImageMemory(m_DenoisingFilter->GetOutput(),m_DenoisedImage);
    m_DenoisedImage->SetPropertyList(m_Image->GetPropertyList()->Clone());

    MITK_ASSERT_EQUAL( m_DenoisedImage, m_ReferenceImage, "NLMr should always return the same result.");
  }

  void Denoise_NLMv_shouldReturnTrue()
  {
    std::string referenceImagePath = GetTestDataFilePath("DiffusionImaging/Denoising/test_multi_NLMv.dwi");
    m_ReferenceImage = mitk::IOUtil::Load<mitk::Image>(referenceImagePath);
    m_DenoisingFilter->SetUseRicianAdaption(false);
    m_DenoisingFilter->SetUseJointInformation(true);
    try
    {
      m_DenoisingFilter->Update();
    }
    catch(std::exception& e)
    {
      MITK_ERROR << e.what();
    }

    mitk::GrabItkImageMemory(m_DenoisingFilter->GetOutput(),m_DenoisedImage);
    m_DenoisedImage->SetPropertyList(m_Image->GetPropertyList()->Clone());

    MITK_ASSERT_EQUAL( m_DenoisedImage, m_ReferenceImage, "NLMv should always return the same result.");
  }

  void Denoise_NLMvr_shouldReturnTrue()
  {
    std::string referenceImagePath = GetTestDataFilePath("DiffusionImaging/Denoising/test_multi_NLMvr.dwi");
    m_ReferenceImage = mitk::IOUtil::Load<mitk::Image>(referenceImagePath);

    m_DenoisingFilter->SetUseRicianAdaption(true);
    m_DenoisingFilter->SetUseJointInformation(true);
    try
    {
      m_DenoisingFilter->Update();
    }
    catch(std::exception& e)
    {
      MITK_ERROR << e.what();
    }

    mitk::GrabItkImageMemory(m_DenoisingFilter->GetOutput(),m_DenoisedImage);
    m_DenoisedImage->SetPropertyList(m_Image->GetPropertyList()->Clone());

    MITK_ASSERT_EQUAL( m_DenoisedImage, m_ReferenceImage, "NLMvr should always return the same result.");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkNonLocalMeansDenoising)

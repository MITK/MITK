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

#include <mitkImage.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkImageGenerator.h>
#include <mitkImageCast.h>

#include <mitkOclBinaryThresholdImageFilter.h>
#include <mitkException.h>

//ITK Filter as reference computation
#include <itkBinaryThresholdImageFilter.h>

class mitkOclBinaryThresholdImageFilterTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkOclBinaryThresholdImageFilterTestSuite);
  MITK_TEST(SetInput_2DImage_ThrowsException);
  MITK_TEST(GenerateData_3DImage_CompareToReference);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().
    */
  mitk::OclBinaryThresholdImageFilter::Pointer m_oclBinaryFilter;

  mitk::Image::Pointer m_Random2DImage;
  mitk::Image::Pointer m_Random3DImage;
  mitk::Image::Pointer m_ReferenceImage;

public:

  /**
    * @brief Setup a recorder including a device. Here, we use a player, because in an automatic test
    * hardware is not useful.
    */
  void setUp()
  {
    //Random input images
    m_Random3DImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(119, 204, 52, 1,  // dimension
                                                                               1.0f, 1.0f, 1.0f, // spacing
                                                                               255, 0); // max, min

    m_Random2DImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(119, 204, 0, 0,   // dimension
                                                                               1.0f, 1.0f, 1.0f, // spacing
                                                                               255, 0); // max, min
    m_oclBinaryFilter = mitk::OclBinaryThresholdImageFilter::New();
  }

  void tearDown()
  {
    m_oclBinaryFilter = NULL;
  }

  void SetInput_2DImage_ThrowsException()
  {
    CPPUNIT_ASSERT_THROW( m_oclBinaryFilter->SetInput( m_Random2DImage ), mitk::Exception );
  }

  void GenerateData_3DImage_CompareToReference()
  {
    int upperThr = 255;
    int lowerThr = 60;

    int outsideVal = 0;
    int insideVal = 100;

    us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
    OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);
    resources->GetContext(); //todo why do i need to call this before GetMaximumImageSize()?
    if(resources->GetMaximumImageSize(2, CL_MEM_OBJECT_IMAGE3D) == 0)
    {
      //GPU device does not support 3D images. Skip this test.
      MITK_INFO << "Skipping test.";
      return;
    }

    try{

      m_oclBinaryFilter->SetInput( m_Random3DImage );
      m_oclBinaryFilter->SetUpperThreshold( upperThr );
      m_oclBinaryFilter->SetLowerThreshold( lowerThr );
      m_oclBinaryFilter->SetOutsideValue( outsideVal );
      m_oclBinaryFilter->SetInsideValue( insideVal );
      m_oclBinaryFilter->Update();

      mitk::Image::Pointer outputImage = mitk::Image::New();
      outputImage = m_oclBinaryFilter->GetOutput();

      // reference computation
      //This is not optimal here, but since we use a random image
      //we cannot know the reference image at this point.
      typedef itk::Image< unsigned char, 3> ImageType;
      typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > ThresholdFilterType;

      ImageType::Pointer itkInputImage = ImageType::New();
      CastToItkImage( m_Random3DImage, itkInputImage );

      ThresholdFilterType::Pointer refThrFilter = ThresholdFilterType::New();
      refThrFilter->SetInput( itkInputImage );
      refThrFilter->SetLowerThreshold( lowerThr );
      refThrFilter->SetUpperThreshold( upperThr );
      refThrFilter->SetOutsideValue( outsideVal );
      refThrFilter->SetInsideValue( insideVal );
      refThrFilter->Update();
      mitk::Image::Pointer referenceImage = mitk::Image::New();
      mitk::CastToMitkImage(refThrFilter->GetOutput(), referenceImage);

      MITK_ASSERT_EQUAL( referenceImage, outputImage,
                         "OclBinaryThresholdFilter should be equal to regular itkBinaryThresholdImageFilter.");
    }
    catch(mitk::Exception &e)
    {
      std::string errorMessage = "Caught unexpected exception ";
      errorMessage.append(e.what());
      CPPUNIT_FAIL(errorMessage.c_str());
    }

  }
};

MITK_TEST_SUITE_REGISTRATION(mitkOclBinaryThresholdImageFilter)

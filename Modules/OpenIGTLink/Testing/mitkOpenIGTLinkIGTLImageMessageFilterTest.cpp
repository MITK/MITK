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

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkImageToIGTLMessageFilter.h>
#include <mitkImageGenerator.h>

#include <igtlImageMessage.h>

static unsigned int MIN_DIM = 1u;
static unsigned int SMALL_DIM = 10u;
static unsigned int MEDIUM_DIM = 300u;
static unsigned int BIG_DIM = 3000u;

class mitkOpenIGTLinkIGTLImageMessageFilterTestSuite : public mitk::TestFixture {
  CPPUNIT_TEST_SUITE(mitkOpenIGTLinkIGTLImageMessageFilterTestSuite);
  MITK_TEST(TestMinimumImage);
  MITK_TEST(TestSmallImage);
  MITK_TEST(TestMediumImage);
  MITK_TEST(TestLargeImage);
  CPPUNIT_TEST_SUITE_END();

public:

  mitk::ImageToIGTLMessageFilter::Pointer m_ImageToIGTLMessageFilter;
  mitk::Image::Pointer m_TestImage;

  void setUp() override
  {
    m_ImageToIGTLMessageFilter = mitk::ImageToIGTLMessageFilter::New();
  }

  void tearDown() override
  {
    m_ImageToIGTLMessageFilter = NULL;
    m_TestImage = NULL;
  }

  void TestMinimumImage()
  {
    Equal_ContentOfIGTLImageMessageAndMitkImage_True(MIN_DIM);
  }

  void TestSmallImage()
  {
    Equal_ContentOfIGTLImageMessageAndMitkImage_True(SMALL_DIM);
  }

  void TestMediumImage()
  {
    Equal_ContentOfIGTLImageMessageAndMitkImage_True(MEDIUM_DIM);
  }

  void TestLargeImage()
  {
    Equal_ContentOfIGTLImageMessageAndMitkImage_True(BIG_DIM);
  }

  /**
  * This test takes a generated gradient mitk image. Then an IGTL Message is produced
  * using the ImageToIGTLMessageFilter. In the end it is tested, wether the image data in both images is equivalent.
  */
  void Equal_ContentOfIGTLImageMessageAndMitkImage_True(unsigned int dim)
  {
    m_TestImage = mitk::ImageGenerator::GenerateGradientImage<unsigned char>(dim, dim, 1u);

    m_ImageToIGTLMessageFilter->SetInput(m_TestImage);

    m_ImageToIGTLMessageFilter->GenerateData();

    mitk::IGTLMessage::Pointer resultMessage = m_ImageToIGTLMessageFilter->GetOutput();

    CPPUNIT_ASSERT_MESSAGE("Output of ImageToIGTLMessageFilter was null", resultMessage != nullptr);

    igtl::MessageBase::Pointer msgBase = resultMessage->GetMessage();
    igtl::ImageMessage* igtlImageMessage = (igtl::ImageMessage*)(msgBase.GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Output of ImageToIGTLMessageFilter was not of type igtl::ImageMessage", igtlImageMessage != nullptr);

    const void* outputBuffer = igtlImageMessage->GetScalarPointer();

    CPPUNIT_ASSERT_MESSAGE("Output Buffer was null", outputBuffer != nullptr);

    void* inputBuffer = m_TestImage->GetVolumeData()->GetData();

    CPPUNIT_ASSERT_MESSAGE("Input Buffer was null", inputBuffer != nullptr);

    CPPUNIT_ASSERT_MESSAGE("Images were not identical", memcmp(inputBuffer, outputBuffer, dim*dim) == 0);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkOpenIGTLinkIGTLImageMessageFilter)

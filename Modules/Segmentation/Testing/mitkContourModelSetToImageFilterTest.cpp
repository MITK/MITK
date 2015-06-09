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
#include <mitkIOUtil.h>
#include <mitkContourModelSet.h>
#include <mitkContourModelSetToImageFilter.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkContourModelSetToImageFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkContourModelSetToImageFilterTestSuite);
  MITK_TEST(TestFillContourSetIntoImage);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::ContourModelSetToImageFilter::Pointer m_ContourFiller;

public:

  void setUp() override
  {
    m_ContourFiller = mitk::ContourModelSetToImageFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Failed to initialize ContourModelSetToImageFilter", m_ContourFiller.IsNotNull());
  }

  void TestFillContourSetIntoImage()
  {
    mitk::Image::Pointer refImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("ContourModel-Data/RefImage.nrrd"));
    CPPUNIT_ASSERT_MESSAGE("Failed to load reference image", refImage.IsNotNull());
    std::vector<itk::SmartPointer<mitk::BaseData> > readerOutput;
    readerOutput = mitk::IOUtil::Load(GetTestDataFilePath("ContourModel-Data/Contours.cnt_set"));
    mitk::ContourModelSet::Pointer cnt_set = dynamic_cast<mitk::ContourModelSet*>(readerOutput.at(0).GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Failed to load contours", cnt_set.IsNotNull());

    m_ContourFiller->SetImage(refImage);
    m_ContourFiller->SetInput(cnt_set);
    m_ContourFiller->Update();
    mitk::Image::Pointer filledImage = m_ContourFiller->GetOutput();

    filledImage->DisconnectPipeline();

    MITK_ASSERT_EQUAL(refImage, filledImage, "Error filling contours into image");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkContourModelSetToImageFilter)

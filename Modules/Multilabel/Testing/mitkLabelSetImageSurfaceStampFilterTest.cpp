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

#include <mitkImageStatisticsHolder.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkLabelSetImageSurfaceStampFilter.h>
#include <mitkImageGenerator.h>
#include <mitkEqual.h>

class mitkLabelSetImageSurfaceStampFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetImageSurfaceStampFilterTestSuite);

  MITK_TEST(Result_Match_Expectation);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::LabelSetImage::Pointer m_LabelSetImage;
  mitk::Surface::Pointer m_Surface;

public:

  void setUp() override
  {
    mitk::Image::Pointer regularImage = mitk::ImageGenerator::GenerateRandomImage<int>(50,50,50,1,1,1,1,0.3,0.2);
    m_LabelSetImage = dynamic_cast<mitk::LabelSetImage*>(mitk::IOUtil::LoadBaseData(GetTestDataFilePath("Multilabel/EmptyMultiLabelSegmentation.nrrd")).GetPointer());
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    mitk::Label::PixelType value1 = 1;
    label1->SetValue(value1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    mitk::Label::PixelType value2 = 2;
    label2->SetValue(value2);

    m_LabelSetImage->GetActiveLabelSet()->AddLabel(label1);
    m_LabelSetImage->GetActiveLabelSet()->AddLabel(label2);

    m_Surface =  mitk::IOUtil::LoadSurface(GetTestDataFilePath("BallBinary30x30x30Reference.vtp"));
  }

  void tearDown() override
  {
    // Delete LabelSetImage
    m_LabelSetImage = 0;
  }

  void Result_Match_Expectation()
  {
    mitk::LabelSetImageSurfaceStampFilter::Pointer filter = mitk::LabelSetImageSurfaceStampFilter::New();
    filter->SetSurface(m_Surface);
    filter->SetForceOverwrite(true);
    filter->SetInput(m_LabelSetImage);
    filter->Update();
    mitk::LabelSetImage::Pointer result = dynamic_cast<mitk::LabelSetImage*>(filter->GetOutput());
    mitk::IOUtil::Save(result,"d:/tmp/output.nrrd");

    mitk::LabelSetImage::Pointer expectedResult =  dynamic_cast<mitk::LabelSetImage*>(mitk::IOUtil::LoadBaseData(GetTestDataFilePath("Multilabel/StampResultBasedOnEmptyML.nrrd")).GetPointer());
    mitk::LabelSetImage::Pointer expectedResult2 =  dynamic_cast<mitk::LabelSetImage*>(mitk::IOUtil::LoadBaseData(GetTestDataFilePath("Multilabel/StampResultBasedOnEmptyML.nrrd")).GetPointer());
    mitk::IOUtil::Save(expectedResult,"d:/tmp/output2.nrrd");

    mitk::Image::Pointer p1 = dynamic_cast<mitk::Image*>(expectedResult.GetPointer());
    mitk::Image::Pointer p2 = dynamic_cast<mitk::Image*>(expectedResult2.GetPointer());

    MITK_INFO << "Equality: " << mitk::Equal(p1, p2,10,true);
    MITK_ASSERT_EQUAL(expectedResult, result, "Result after stamping should be equal to the saved version");
  }

  // Reduce contours with nth point
  void TestGetActiveLabel()
  {
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSetImageSurfaceStampFilter)
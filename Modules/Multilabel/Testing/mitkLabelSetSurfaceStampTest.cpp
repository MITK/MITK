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

#include <mitkLabelSetSurfaceStamp.h>
#include <mitkImageGenerator.h>

class mitkLabelSetSurfaceStampTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetSurfaceStampTestSuite);

  MITK_TEST(testIfPossible);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::LabelSetImage::Pointer m_LabelSetImage;

public:

  void setUp() override
  {
    mitk::Image::Pointer regularImage = mitk::ImageGenerator::GenerateRandomImage<int>(50,50,50,1,1,1,1,0.3,0.2);
    unsigned int dimensions[3] = {50,50,50};
    m_LabelSetImage = dynamic_cast<mitk::LabelSetImage*>(mitk::IOUtil::LoadBaseData("d:/tmp/output1.nrrd").GetPointer());
  }

  void tearDown() override
  {
    // Delete LabelSetImage
    m_LabelSetImage = 0;
  }

  void testIfPossible()
  {
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    mitk::Label::PixelType value1 = 1;
    label1->SetValue(value1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    mitk::Label::PixelType value2 = 2;
    label2->SetValue(value2);

    //mitk::IOUtil::Save(m_LabelSetImage,"d:/tmp/output1.nrrd");
    m_LabelSetImage->GetActiveLabelSet()->AddLabel(label1);
    m_LabelSetImage->GetActiveLabelSet()->AddLabel(label2);

    //m_LabelSetImage = dynamic_cast<mitk::LabelSetImage*>(mitk::IOUtil::LoadBaseData("d:/tmp/output1.nrrd").GetPointer());
    mitk::Surface::Pointer surface = mitk::IOUtil::LoadSurface(GetTestDataFilePath("BallBinary30x30x30Reference.vtp"));
    mitk::LabelSetSurfaceStamp::Pointer filter = mitk::LabelSetSurfaceStamp::New();
    filter->SetSurface(surface);
    filter->SetForceOverwrite(true);
    filter->SetInput(m_LabelSetImage);
    filter->Update();
    mitk::IOUtil::Save(filter->GetOutput(),"d:/tmp/output.nrrd");
  }

  // Reduce contours with nth point
  void TestGetActiveLabel()
  {
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSetSurfaceStamp)
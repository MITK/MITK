/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

//MITK includes
#include "mitkConcentrationCurveGenerator.h"
#include "mitkTestDynamicImageGenerator.h"
#include "mitkImagePixelReadAccessor.h"


class mitkConvertSignalToConcentrationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkConvertSignalToConcentrationTestSuite);
  MITK_TEST(GetConvertedImageAbsoluteEnhancementTest);
  MITK_TEST(GetConvertedImageAbsoluteEnhancementAveragedBaselineTest);
  MITK_TEST(GetConvertedImageRelativeEnhancementTest);
  MITK_TEST(GetConvertedImageturboFLASHTest);
  MITK_TEST(GetConvertedImageVFATest);
  MITK_TEST(GetConvertedImageT2Test);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_dynamicImage;
  mitk::Image::Pointer m_PDWImage;
  mitk::Image::Pointer m_convertedImage_0;
  mitk::Image::Pointer m_convertedImage_1;
  mitk::Image::Pointer m_convertedImage_2;
  mitk::Image::Pointer m_convertedImage_3;
  mitk::Image::Pointer m_convertedImage_4;
  mitk::Image::Pointer m_convertedImage_5;

  itk::Index<4> m_idx_0 = { { 0, 0, 0, 0 } };
  itk::Index<4> m_idx_1 = { { 0, 2, 2, 3 } };
  itk::Index<4> m_idx_2 = { { 2, 2, 1, 9 } };
  itk::Index<4> m_idx_3 = { { 0, 3, 2, 1 } };
  itk::Index<4> m_idx_4 = { { 2, 2, 2, 4 } };


public:
  void setUp() override
  {
    m_dynamicImage = mitk::GenerateDynamicTestImageMITK();

    // absolute enhancement
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen_0 =
      mitk::ConcentrationCurveGenerator::New();
    concentrationGen_0->SetDynamicImage(m_dynamicImage);
    concentrationGen_0->SetAbsoluteSignalEnhancement(true);
    concentrationGen_0->SetFactor(1.0);
    concentrationGen_0->SetBaselineStartTimeStep(0);
    concentrationGen_0->SetBaselineEndTimeStep(0);
    m_convertedImage_0 = concentrationGen_0->GetConvertedImage();

    // average baseline, absolute enhancement
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen_1 =
      mitk::ConcentrationCurveGenerator::New();
    concentrationGen_1->SetDynamicImage(m_dynamicImage);
    concentrationGen_1->SetAbsoluteSignalEnhancement(true);
    concentrationGen_1->SetFactor(1.0);
    concentrationGen_1->SetBaselineStartTimeStep(1);
    concentrationGen_1->SetBaselineEndTimeStep(3);
    m_convertedImage_1 = concentrationGen_1->GetConvertedImage();

    // relative enhancement
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen_2 =
      mitk::ConcentrationCurveGenerator::New();
    concentrationGen_2->SetDynamicImage(m_dynamicImage);
    concentrationGen_2->SetRelativeSignalEnhancement(true);
    concentrationGen_2->SetFactor(1.0);
    concentrationGen_2->SetBaselineStartTimeStep(0);
    concentrationGen_2->SetBaselineEndTimeStep(0);
    m_convertedImage_2 = concentrationGen_2->GetConvertedImage();

    // turboFLASH
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen_3 =
      mitk::ConcentrationCurveGenerator::New();
    concentrationGen_3->SetDynamicImage(m_dynamicImage);
    concentrationGen_3->SetisTurboFlashSequence(true);
    concentrationGen_3->SetRecoveryTime(1.0);
    concentrationGen_3->SetRelaxationTime(1.0);
    concentrationGen_3->SetRelaxivity(1.0);
    concentrationGen_3->SetBaselineStartTimeStep(0);
    concentrationGen_3->SetBaselineEndTimeStep(0);
    m_convertedImage_3 = concentrationGen_3->GetConvertedImage();

    // Variable flip angle
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen_4 =
      mitk::ConcentrationCurveGenerator::New();
    m_PDWImage = mitk::GenerateTestFrame(1.0);
    concentrationGen_4->SetDynamicImage(m_dynamicImage);
    concentrationGen_4->SetUsingT1Map(true);
    concentrationGen_4->SetRecoveryTime(1.0);
    concentrationGen_4->SetRelaxivity(1.0);
    concentrationGen_4->SetT10Image(m_PDWImage);
    concentrationGen_4->SetFlipAngle(1.0);
    concentrationGen_4->SetBaselineStartTimeStep(0);
    concentrationGen_4->SetBaselineEndTimeStep(0);
    m_convertedImage_4 = concentrationGen_4->GetConvertedImage();

    // T2
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen_5 =
      mitk::ConcentrationCurveGenerator::New();
    concentrationGen_5->SetDynamicImage(m_dynamicImage);
    concentrationGen_5->SetisT2weightedImage(true);
    concentrationGen_5->SetT2Factor(1.0);
    concentrationGen_5->SetT2EchoTime(1.0);
    concentrationGen_5->SetBaselineStartTimeStep(0);
    concentrationGen_5->SetBaselineEndTimeStep(0);
    m_convertedImage_5 = concentrationGen_5->GetConvertedImage();
  }

  void tearDown() override
  {
  }

  void GetConvertedImageAbsoluteEnhancementTest()
  {
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage_0, m_convertedImage_0->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 0 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 0 at pixel m_idx_1.", mitk::Equal(90.0, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 0 at pixel m_idx_2.", mitk::Equal(360.0, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 0 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 0 at pixel m_idx_4.", mitk::Equal(160.0, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

  void GetConvertedImageAbsoluteEnhancementAveragedBaselineTest()
  {
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage_1, m_convertedImage_1->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 1 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 1 at pixel m_idx_1.", mitk::Equal(30.0, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 1 at pixel m_idx_2.", mitk::Equal(280.0, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 1 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 1 at pixel m_idx_4.", mitk::Equal(80.0, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

  void GetConvertedImageRelativeEnhancementTest()
  {
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage_2, m_convertedImage_2->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 2 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 2 at pixel m_idx_1.", mitk::Equal(3.461538, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 2 at pixel m_idx_2.", mitk::Equal(20.0, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 2 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 2 at pixel m_idx_4.", mitk::Equal(5.714286, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

  void GetConvertedImageturboFLASHTest()
  {
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage_3, m_convertedImage_3->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 3 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 3 at pixel m_idx_1.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 3 at pixel m_idx_2.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 3 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 3 at pixel m_idx_4.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

  void GetConvertedImageVFATest()
  {
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage_4, m_convertedImage_4->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 4 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 4 at pixel m_idx_1.", mitk::Equal(2.956868, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 4 at pixel m_idx_2.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 4 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 4 at pixel m_idx_4.", mitk::Equal(3.989374, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

  void GetConvertedImageT2Test()
  {
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage_5, m_convertedImage_5->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 5 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 5 at pixel m_idx_1.", mitk::Equal(-1.495494, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 5 at pixel m_idx_2.", mitk::Equal(-3.044522, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 5 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 5 at pixel m_idx_4.", mitk::Equal(-1.904237, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

};
MITK_TEST_SUITE_REGISTRATION(mitkConvertSignalToConcentration)



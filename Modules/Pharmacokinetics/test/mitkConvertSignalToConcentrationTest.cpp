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

  itk::Index<4> m_idx_0 = { { 0, 0, 0, 0 } };
  itk::Index<4> m_idx_1 = { { 0, 2, 2, 3 } };
  itk::Index<4> m_idx_2 = { { 2, 2, 1, 9 } };
  itk::Index<4> m_idx_3 = { { 0, 3, 2, 1 } };
  itk::Index<4> m_idx_4 = { { 2, 2, 2, 4 } };


public:
  void setUp() override
  {
    m_dynamicImage = mitk::GenerateDynamicTestImageMITK();
  }

  void tearDown() override
  {
  }

  void GetConvertedImageAbsoluteEnhancementTest()
  {
    mitk::Image::Pointer m_convertedImage;
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen = mitk::ConcentrationCurveGenerator::New();
    concentrationGen->SetDynamicImage(m_dynamicImage);
    concentrationGen->SetAbsoluteSignalEnhancement(true);
    concentrationGen->SetFactor(1.0);
    concentrationGen->SetBaselineStartTimeStep(0);
    concentrationGen->SetBaselineEndTimeStep(0);
    m_convertedImage = concentrationGen->GetConvertedImage();
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage, m_convertedImage->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 0 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 0 at pixel m_idx_1.", mitk::Equal(90.0, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 0 at pixel m_idx_2.", mitk::Equal(360.0, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 0 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 0 at pixel m_idx_4.", mitk::Equal(160.0, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

  void GetConvertedImageAbsoluteEnhancementAveragedBaselineTest()
  {
    mitk::Image::Pointer convertedImage;
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen = mitk::ConcentrationCurveGenerator::New();
    concentrationGen->SetDynamicImage(m_dynamicImage);
    concentrationGen->SetAbsoluteSignalEnhancement(true);
    concentrationGen->SetFactor(1.0);
    concentrationGen->SetBaselineStartTimeStep(1);
    concentrationGen->SetBaselineEndTimeStep(3);
    convertedImage = concentrationGen->GetConvertedImage();
    mitk::ImagePixelReadAccessor<double, 4> readAccess(convertedImage, convertedImage->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 1 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 1 at pixel m_idx_1.", mitk::Equal(30.0, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 1 at pixel m_idx_2.", mitk::Equal(280.0, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 1 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 1 at pixel m_idx_4.", mitk::Equal(80.0, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

  void GetConvertedImageRelativeEnhancementTest()
  {
    mitk::Image::Pointer convertedImage;
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen = mitk::ConcentrationCurveGenerator::New();
    concentrationGen->SetDynamicImage(m_dynamicImage);
    concentrationGen->SetRelativeSignalEnhancement(true);
    concentrationGen->SetFactor(1.0);
    concentrationGen->SetBaselineStartTimeStep(0);
    concentrationGen->SetBaselineEndTimeStep(0);
    convertedImage = concentrationGen->GetConvertedImage();

    mitk::ImagePixelReadAccessor<double, 4> readAccess(convertedImage, convertedImage->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 2 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 2 at pixel m_idx_1.", mitk::Equal(3.461538, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 2 at pixel m_idx_2.", mitk::Equal(20.0, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 2 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 2 at pixel m_idx_4.", mitk::Equal(5.714286, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

  void GetConvertedImageturboFLASHTest()
  {
    mitk::Image::Pointer convertedImage;
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen = mitk::ConcentrationCurveGenerator::New();
    concentrationGen->SetDynamicImage(m_dynamicImage);
    concentrationGen->SetisTurboFlashSequence(true);
    concentrationGen->SetRecoveryTime(1.0);
    concentrationGen->SetRelaxationTime(1.0);
    concentrationGen->SetRelaxivity(1.0);
    concentrationGen->SetBaselineStartTimeStep(0);
    concentrationGen->SetBaselineEndTimeStep(0);
    convertedImage = concentrationGen->GetConvertedImage();

    mitk::ImagePixelReadAccessor<double, 4> readAccess(convertedImage, convertedImage->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 3 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 3 at pixel m_idx_1.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 3 at pixel m_idx_2.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 3 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 3 at pixel m_idx_4.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

  void GetConvertedImageVFATest()
  {
    mitk::Image::Pointer PDWImage;
    mitk::Image::Pointer convertedImage;
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen = mitk::ConcentrationCurveGenerator::New();
    PDWImage = mitk::GenerateTestFrame(1.0);
    concentrationGen->SetDynamicImage(m_dynamicImage);
    concentrationGen->SetUsingT1Map(true);
    concentrationGen->SetRecoveryTime(1.0);
    concentrationGen->SetRelaxivity(1.0);
    concentrationGen->SetT10Image(PDWImage);
    concentrationGen->SetFlipAngle(1.0);
    concentrationGen->SetBaselineStartTimeStep(0);
    concentrationGen->SetBaselineEndTimeStep(0);
    convertedImage = concentrationGen->GetConvertedImage();

    mitk::ImagePixelReadAccessor<double, 4> readAccess(convertedImage, convertedImage->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 4 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 4 at pixel m_idx_1.", mitk::Equal(2.956868, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 4 at pixel m_idx_2.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 4 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 4 at pixel m_idx_4.", mitk::Equal(3.989374, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

  void GetConvertedImageT2Test()
  {
    mitk::Image::Pointer convertedImage;
    mitk::ConcentrationCurveGenerator::Pointer concentrationGen = mitk::ConcentrationCurveGenerator::New();
    concentrationGen->SetDynamicImage(m_dynamicImage);
    concentrationGen->SetisT2weightedImage(true);
    concentrationGen->SetT2Factor(1.0);
    concentrationGen->SetT2EchoTime(1.0);
    concentrationGen->SetBaselineStartTimeStep(0);
    concentrationGen->SetBaselineEndTimeStep(0);
    convertedImage = concentrationGen->GetConvertedImage();
    mitk::ImagePixelReadAccessor<double, 4> readAccess(convertedImage, convertedImage->GetSliceData(4));

    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 5 at pixel m_idx_0.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_0), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 5 at pixel m_idx_1.", mitk::Equal(-1.495494, readAccess.GetPixelByIndex(m_idx_1), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 5 at pixel m_idx_2.", mitk::Equal(-3.044522, readAccess.GetPixelByIndex(m_idx_2), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 5 at pixel m_idx_3.", mitk::Equal(0.0, readAccess.GetPixelByIndex(m_idx_3), 1e-6, true) == true);
    CPPUNIT_ASSERT_MESSAGE("Checking value of converted image 5 at pixel m_idx_4.", mitk::Equal(-1.904237, readAccess.GetPixelByIndex(m_idx_4), 1e-6, true) == true);
  }

};
MITK_TEST_SUITE_REGISTRATION(mitkConvertSignalToConcentration)



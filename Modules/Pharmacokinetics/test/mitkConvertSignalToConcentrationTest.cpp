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
#include "boost/math/constants/constants.hpp"


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
  mitk::Image::Pointer m_convertedImage;
  mitk::ConcentrationCurveGenerator::Pointer m_concentrationGen;
  std::vector <itk::Index<4>> m_testIndices;

public:
  void setUp() override
  {
    m_dynamicImage = mitk::GenerateDynamicTestImageMITK();
    m_concentrationGen = mitk::ConcentrationCurveGenerator::New();
    m_concentrationGen->SetDynamicImage(m_dynamicImage);
    itk::Index<4> testIndex0 = { { 0, 0, 0, 0 } };
    itk::Index<4> testIndex1 = { { 0, 2, 2, 3 } };
    itk::Index<4> testIndex2 = { { 2, 2, 1, 9 } };
    itk::Index<4> testIndex3 = { { 0, 3, 2, 1 } };
    itk::Index<4> testIndex4 = { { 2, 2, 2, 4 } };
    m_testIndices.push_back(testIndex0);
    m_testIndices.push_back(testIndex1);
    m_testIndices.push_back(testIndex2);
    m_testIndices.push_back(testIndex3);
    m_testIndices.push_back(testIndex4);
  }

  void tearDown() override
  {
  }

  void GetConvertedImageAbsoluteEnhancementTest()
  {
    m_concentrationGen->SetAbsoluteSignalEnhancement(true);
    m_concentrationGen->SetFactor(1.0);
    m_concentrationGen->SetBaselineStartTimeStep(0);
    m_concentrationGen->SetBaselineEndTimeStep(0);
    m_convertedImage = m_concentrationGen->GetConvertedImage();
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage, m_convertedImage->GetSliceData(4));
    std::vector <double> refValues;
    refValues.push_back(0.0);
    refValues.push_back(90.0);
    refValues.push_back(360.0);
    refValues.push_back(0.0);
    refValues.push_back(160.0);

    std::stringstream ss;
    for (long unsigned int i = 0; i < m_testIndices.size(); i++)
    {
      ss << "Checking value of image converted using absolute enhancement at test index " << i << ".";
      std::string message = ss.str();
      CPPUNIT_ASSERT_MESSAGE(message, mitk::Equal(refValues.at(i), readAccess.GetPixelByIndex(m_testIndices.at(i)), 1e-6, true) == true);
    }
  }

  void GetConvertedImageAbsoluteEnhancementAveragedBaselineTest()
  {
    m_concentrationGen->SetAbsoluteSignalEnhancement(true);
    m_concentrationGen->SetFactor(1.0);
    m_concentrationGen->SetBaselineStartTimeStep(1);
    m_concentrationGen->SetBaselineEndTimeStep(3);
    m_convertedImage = m_concentrationGen->GetConvertedImage();
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage, m_convertedImage->GetSliceData(4));
    std::vector <double> refValues;
    refValues.push_back(0.0);
    refValues.push_back(30.0);
    refValues.push_back(280.0);
    refValues.push_back(0.0);
    refValues.push_back(80.0);

    std::stringstream ss;
    for (long unsigned int i = 0; i < m_testIndices.size(); i++)
    {
      ss << "Checking value of image converted using absolute enhancement with averaged baseline at test index " << i << ".";
      std::string message = ss.str();
      CPPUNIT_ASSERT_MESSAGE(message, mitk::Equal(refValues.at(i), readAccess.GetPixelByIndex(m_testIndices.at(i)), 1e-6, true) == true);
    }
 }

  void GetConvertedImageRelativeEnhancementTest()
  {
    m_concentrationGen->SetRelativeSignalEnhancement(true);
    m_concentrationGen->SetFactor(1.0);
    m_concentrationGen->SetBaselineStartTimeStep(0);
    m_concentrationGen->SetBaselineEndTimeStep(0);
    m_convertedImage = m_concentrationGen->GetConvertedImage();
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage, m_convertedImage->GetSliceData(4));
    std::vector <double> refValues;
    refValues.push_back(0.0);
    refValues.push_back(3.461538);
    refValues.push_back(20.0);
    refValues.push_back(0.0);
    refValues.push_back(5.714286);

    std::stringstream ss;
    for (long unsigned int i = 0; i < m_testIndices.size(); i++)
    {
      ss << "Checking value of image converted using relative enhancement at test index " << i << ".";
      std::string message = ss.str();
      CPPUNIT_ASSERT_MESSAGE(message, mitk::Equal(refValues.at(i), readAccess.GetPixelByIndex(m_testIndices.at(i)), 1e-6, true) == true);
    }
 }

  void GetConvertedImageturboFLASHTest()
  {
    m_concentrationGen->SetisTurboFlashSequence(true);
    m_concentrationGen->SetRecoveryTime(1.0);
    m_concentrationGen->SetRelaxationTime(1.0);
    m_concentrationGen->SetRelaxivity(1.0);
    m_concentrationGen->SetBaselineStartTimeStep(0);
    m_concentrationGen->SetBaselineEndTimeStep(0);
    m_convertedImage = m_concentrationGen->GetConvertedImage();
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage, m_convertedImage->GetSliceData(4));
    std::vector <double> refValues;
    refValues.push_back(0.0);
    refValues.push_back(0.0);
    refValues.push_back(0.0);
    refValues.push_back(0.0);
    refValues.push_back(0.0);

    std::stringstream ss;
    for (long unsigned int i = 0; i < m_testIndices.size(); i++)
    {
      ss << "Checking value of image converted using the turboFLASH model at test index " << i << ".";
      std::string message = ss.str();
      CPPUNIT_ASSERT_MESSAGE(message, mitk::Equal(refValues.at(i), readAccess.GetPixelByIndex(m_testIndices.at(i)), 1e-6, true) == true);
    }
  }

  void GetConvertedImageVFATest()
  {
    mitk::Image::Pointer PDWImage;
    PDWImage = mitk::GenerateTestFrame(2.0);
    m_concentrationGen->SetUsingT1Map(true);
    m_concentrationGen->SetRepetitionTime(1.0);
    m_concentrationGen->SetRelaxivity(1.0/1000.0);
    m_concentrationGen->SetPDWImage(PDWImage);
    m_concentrationGen->SetFlipAngle(2.0 / 360 * 2 * boost::math::constants::pi<double>());
    m_concentrationGen->SetFlipAnglePDW(1.0 / 360 * 2 * boost::math::constants::pi<double>());
    m_concentrationGen->SetBaselineStartTimeStep(0);
    m_concentrationGen->SetBaselineEndTimeStep(0);
    m_convertedImage = m_concentrationGen->GetConvertedImage();

    mitk::ImagePixelReadAccessor<double, 3> readAccessPDW(PDWImage, PDWImage->GetSliceData(3));
    mitk::ImagePixelReadAccessor<double, 4> readAccessDyn(m_dynamicImage, m_dynamicImage->GetSliceData(4));
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage, m_convertedImage->GetSliceData(4));
    std::vector <double> refValues;
    refValues.push_back(0.0);
    refValues.push_back(7871.5616001);
    refValues.push_back(-1059.012343);
    refValues.push_back(0.0);
    refValues.push_back(-3251.392613);

    std::stringstream ss;
    for (long unsigned int i = 0; i < m_testIndices.size(); i++)
    {
      ss << "Checking value of image converted using the VFA method at test index " << i << ".";
      std::string message = ss.str();
      CPPUNIT_ASSERT_MESSAGE(message, mitk::Equal(refValues.at(i), readAccess.GetPixelByIndex(m_testIndices.at(i)), 1e-6, true) == true);
    }
 }

  void GetConvertedImageT2Test()
  {
    m_concentrationGen->SetisT2weightedImage(true);
    m_concentrationGen->SetT2Factor(1.0);
    m_concentrationGen->SetT2EchoTime(1.0);
    m_concentrationGen->SetBaselineStartTimeStep(0);
    m_concentrationGen->SetBaselineEndTimeStep(0);
    m_convertedImage = m_concentrationGen->GetConvertedImage();
    mitk::ImagePixelReadAccessor<double, 4> readAccess(m_convertedImage, m_convertedImage->GetSliceData(4));
    std::vector <double> refValues;
    refValues.push_back(0.0);
    refValues.push_back(-1.495494);
    refValues.push_back(-3.044522);
    refValues.push_back(0.0);
    refValues.push_back(-1.904237);

    std::stringstream ss;
    for (long unsigned int i = 0; i < m_testIndices.size(); i++)
    {
      ss << "Checking value of converted T2 image at test index " << i << ".";
      std::string message = ss.str();
      CPPUNIT_ASSERT_MESSAGE(message, mitk::Equal(refValues.at(i), readAccess.GetPixelByIndex(m_testIndices.at(i)), 1e-6, true) == true);
    }
 }

};
MITK_TEST_SUITE_REGISTRATION(mitkConvertSignalToConcentration)



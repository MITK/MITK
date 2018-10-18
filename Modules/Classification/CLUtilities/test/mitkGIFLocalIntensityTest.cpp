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
#include "mitkIOUtil.h"
#include <cmath>

#include <mitkGIFLocalIntensity.h>

class mitkGIFLocalIntensityTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFLocalIntensityTestSuite);

  MITK_TEST(ImageDescription_PhantomTest_Small);
  MITK_TEST(ImageDescription_PhantomTest_Large);
  MITK_TEST(ImageDescription_PhantomTest_Large_RangeChanged);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_IBSI_Phantom_Image_Small;
  mitk::Image::Pointer m_IBSI_Phantom_Image_Large;
  mitk::Image::Pointer m_IBSI_Phantom_Mask_Small;
  mitk::Image::Pointer m_IBSI_Phantom_Mask_Large;

public:

  void setUp(void) override
  {
    m_IBSI_Phantom_Image_Small = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Small.nrrd"));
    m_IBSI_Phantom_Image_Large = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Large.nrrd"));
    m_IBSI_Phantom_Mask_Small = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Mask_Small.nrrd"));
    m_IBSI_Phantom_Mask_Large = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Mask_Large.nrrd"));
  }

  void ImageDescription_PhantomTest_Small()
  {
    mitk::GIFLocalIntensity::Pointer featureCalculator = mitk::GIFLocalIntensity::New();

    auto featureList = featureCalculator->CalculateFeatures(m_IBSI_Phantom_Image_Small, m_IBSI_Phantom_Mask_Small);

    std::map<std::string, double> results;
    for (auto valuePair : featureList)
    {
      MITK_INFO << valuePair.first << " : " << valuePair.second;
      results[valuePair.first] = valuePair.second;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 2 features.", std::size_t(2), featureList.size());

    // These values are obtained in cooperation with IBSI
    // Reported with an accuracy of 0.1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Local Intensity::Local Intensity Peak with Large IBSI Phantom Image", 2.6, results["Local Intensity::2. Local Intensity Peak"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Local Intensity::Global Intensity Peak with Large IBSI Phantom Image", 3.1, results["Local Intensity::2. Global Intensity Peak"], 0.1);
  }

  void ImageDescription_PhantomTest_Large()
  {
    mitk::GIFLocalIntensity::Pointer featureCalculator = mitk::GIFLocalIntensity::New();

    auto featureList = featureCalculator->CalculateFeatures(m_IBSI_Phantom_Image_Large, m_IBSI_Phantom_Mask_Large);

    std::map<std::string, double> results;
    for (auto valuePair : featureList)
    {
      MITK_INFO << valuePair.first << " : " << valuePair.second;
      results[valuePair.first] = valuePair.second;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 2 features.", std::size_t(2), featureList.size());

    // These values are obtained by running the tool
    // They  might be wrong
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Local Intensity::Local Intensity Peak with Large IBSI Phantom Image", 1.43, results["Local Intensity::2. Local Intensity Peak"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Local Intensity::Global Intensity Peak with Large IBSI Phantom Image", 1.43, results["Local Intensity::2. Global Intensity Peak"], 0.01);
  }

  void ImageDescription_PhantomTest_Large_RangeChanged()
  {
    mitk::GIFLocalIntensity::Pointer featureCalculator = mitk::GIFLocalIntensity::New();

    featureCalculator->SetRange(1);

    auto featureList = featureCalculator->CalculateFeatures(m_IBSI_Phantom_Image_Large, m_IBSI_Phantom_Mask_Large);

    std::map<std::string, double> results;
    for (auto valuePair : featureList)
    {
      MITK_INFO << valuePair.first << " : " << valuePair.second;
      results[valuePair.first] = valuePair.second;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 2 features.", std::size_t(2), featureList.size());

    // These values are obtained by running the tool
    // They  might be wrong
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Local Intensity::Local Intensity Peak with Large IBSI Phantom Image", 6, results["Local Intensity::2. Local Intensity Peak"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Local Intensity::Global Intensity Peak with Large IBSI Phantom Image", 6, results["Local Intensity::2. Global Intensity Peak"], 0.01);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFLocalIntensity )
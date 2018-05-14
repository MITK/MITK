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

#include <mitkGIFGreyLevelDistanceZone.h>

class mitkGIFGreyLevelDistanceZoneTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFGreyLevelDistanceZoneTestSuite );

  MITK_TEST(ImageDescription_PhantomTest_3D);
  MITK_TEST(ImageDescription_PhantomTest_2D);

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

  void ImageDescription_PhantomTest_3D()
  {
    mitk::GIFGreyLevelDistanceZone::Pointer featureCalculator = mitk::GIFGreyLevelDistanceZone::New();

    featureCalculator->SetUseBinsize(true);
    featureCalculator->SetBinsize(1.0);
    featureCalculator->SetUseMinimumIntensity(true);
    featureCalculator->SetUseMaximumIntensity(true);
    featureCalculator->SetMinimumIntensity(0.5);
    featureCalculator->SetMaximumIntensity(6.5);

    auto featureList = featureCalculator->CalculateFeatures(m_IBSI_Phantom_Image_Large, m_IBSI_Phantom_Mask_Large);

    std::map<std::string, double> results;
    for (auto valuePair : featureList)
    {
      MITK_INFO << valuePair.first << " : " << valuePair.second;
      results[valuePair.first] = valuePair.second;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 19 features.", std::size_t(19), featureList.size());

    // These values are obtained with IBSI
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Small Distance Emphasis with Large IBSI Phantom Image", 1, results["Grey Level Distance Zone::Small Distance Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Large Distance Emphasis with Large IBSI Phantom Image", 1, results["Grey Level Distance Zone::Large Distance Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Low Grey Level Emphasis with Large IBSI Phantom Image", 0.253, results["Grey Level Distance Zone::Low Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::High Grey Level Emphasis with Large IBSI Phantom Image", 15.6, results["Grey Level Distance Zone::High Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Small Distance Low Grey Level Emphasis with Large IBSI Phantom Image", 0.253, results["Grey Level Distance Zone::Small Distance Low Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Small Distance High Grey Level Emphasis with Large IBSI Phantom Image", 15.6, results["Grey Level Distance Zone::Small Distance High Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Large Distance Low Grey Level Emphasis with Large IBSI Phantom Image", 0.253, results["Grey Level Distance Zone::Large Distance Low Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Large Distance High Grey Level Emphasis with Large IBSI Phantom Image", 15.6, results["Grey Level Distance Zone::Large Distance High Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Grey Level Non-Uniformity with Large IBSI Phantom Image", 1.4, results["Grey Level Distance Zone::Grey Level Non-Uniformity"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Grey Level Non-Uniformity Normalized with Large IBSI Phantom Image", 0.28, results["Grey Level Distance Zone::Grey Level Non-Uniformity Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Distance Size Non-Uniformity with Large IBSI Phantom Image", 5, results["Grey Level Distance Zone::Distance Size Non-Uniformity"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Distance Size Non-Uniformity Normalized with Large IBSI Phantom Image", 1, results["Grey Level Distance Zone::Distance Size Non-Uniformity Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Zone Percentage with Large IBSI Phantom Image", 0.0676, results["Grey Level Distance Zone::Zone Percentage"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Grey Level Variance with Large IBSI Phantom Image", 2.64, results["Grey Level Distance Zone::Grey Level Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Zone Distance Variance with Large IBSI Phantom Image", 0,   results["Grey Level Distance Zone::Zone Distance Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Zone Distance Entropy with Large IBSI Phantom Image", 1.92, results["Grey Level Distance Zone::Zone Distance Entropy"], 0.01);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone:: with Large IBSI Phantom Image", 0.045, results["Grey Level Distance Zone::"], 0.001);

    // These values are obtained by manually running the tool
    // Values might be wrong.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Grey Level Mean with Large IBSI Phantom Image", 3.6, results["Grey Level Distance Zone::Grey Level Mean"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Zone Distance Mean with Large IBSI Phantom Image", 1, results["Grey Level Distance Zone::Zone Distance Mean"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Grey Level Entropy with Large IBSI Phantom Image", 1.92, results["Grey Level Distance Zone::Grey Level Entropy"], 0.01);
  }

  void ImageDescription_PhantomTest_2D()
  {
    mitk::GIFGreyLevelDistanceZone::Pointer featureCalculator = mitk::GIFGreyLevelDistanceZone::New();

    featureCalculator->SetUseBinsize(true);
    featureCalculator->SetBinsize(1.0);
    featureCalculator->SetUseMinimumIntensity(true);
    featureCalculator->SetUseMaximumIntensity(true);
    featureCalculator->SetMinimumIntensity(0.5);
    featureCalculator->SetMaximumIntensity(6.5);

    auto featureList = featureCalculator->CalculateFeaturesSlicewise(m_IBSI_Phantom_Image_Large, m_IBSI_Phantom_Mask_Large, 2);

    std::map<std::string, double> results;
    for (auto valuePair : featureList)
    {
      MITK_INFO << valuePair.first << " : " << valuePair.second;
      results[valuePair.first] = valuePair.second;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 114 features.", std::size_t(114), featureList.size());

    // These values are obtained with IBSI
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Small Distance Emphasis with Large IBSI Phantom Image", 0.946, results["SliceWise Mean Grey Level Distance Zone::Small Distance Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Large Distance Emphasis with Large IBSI Phantom Image", 1.21, results["SliceWise Mean Grey Level Distance Zone::Large Distance Emphasis"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Low Grey Level Emphasis with Large IBSI Phantom Image", 0.371, results["SliceWise Mean Grey Level Distance Zone::Low Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::High Grey Level Emphasis with Large IBSI Phantom Image", 16.4, results["SliceWise Mean Grey Level Distance Zone::High Grey Level Emphasis"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Small Distance Low Grey Level Emphasis with Large IBSI Phantom Image", 0.367, results["SliceWise Mean Grey Level Distance Zone::Small Distance Low Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Small Distance High Grey Level Emphasis with Large IBSI Phantom Image", 15.2, results["SliceWise Mean Grey Level Distance Zone::Small Distance High Grey Level Emphasis"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Large Distance Low Grey Level Emphasis with Large IBSI Phantom Image", 0.386, results["SliceWise Mean Grey Level Distance Zone::Large Distance Low Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Large Distance High Grey Level Emphasis with Large IBSI Phantom Image", 21.3, results["SliceWise Mean Grey Level Distance Zone::Large Distance High Grey Level Emphasis"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Grey Level Non-Uniformity with Large IBSI Phantom Image", 1.41, results["SliceWise Mean Grey Level Distance Zone::Grey Level Non-Uniformity"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Grey Level Non-Uniformity Normalized with Large IBSI Phantom Image", 0.323, results["SliceWise Mean Grey Level Distance Zone::Grey Level Non-Uniformity Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Distance Size Non-Uniformity with Large IBSI Phantom Image", 3.79, results["SliceWise Mean Grey Level Distance Zone::Distance Size Non-Uniformity"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Distance Size Non-Uniformity Normalized with Large IBSI Phantom Image", 0.898, results["SliceWise Mean Grey Level Distance Zone::Distance Size Non-Uniformity Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Zone Percentage with Large IBSI Phantom Image", 0.24, results["SliceWise Mean Grey Level Distance Zone::Zone Percentage"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Grey Level Variance with Large IBSI Phantom Image", 3.97, results["SliceWise Mean Grey Level Distance Zone::Grey Level Variance"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Zone Distance Variance with Large IBSI Phantom Image", 0.051, results["SliceWise Mean Grey Level Distance Zone::Zone Distance Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Zone Distance Entropy with Large IBSI Phantom Image", 1.73, results["SliceWise Mean Grey Level Distance Zone::Zone Distance Entropy"], 0.01);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone:: with Large IBSI Phantom Image", 0.045, results["Grey Level Distance Zone::"], 0.001);

    // These values are obtained by manually running the tool
    // Values might be wrong.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Grey Level Mean with Large IBSI Phantom Image", 3.526, results["SliceWise Mean Grey Level Distance Zone::Grey Level Mean"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Zone Distance Mean with Large IBSI Phantom Image", 1.071, results["SliceWise Mean Grey Level Distance Zone::Zone Distance Mean"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Distance Zone::Grey Level Entropy with Large IBSI Phantom Image", 1.732, results["SliceWise Mean Grey Level Distance Zone::Grey Level Entropy"], 0.01);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFGreyLevelDistanceZone )
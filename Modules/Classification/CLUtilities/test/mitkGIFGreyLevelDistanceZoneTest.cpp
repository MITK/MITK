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
  //MITK_TEST(ImageDescription_PhantomTest_2D);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_IBSI_Phantom_Image_Small;
  mitk::Image::Pointer m_IBSI_Phantom_Image_Large;
  mitk::Image::Pointer m_IBSI_Phantom_Mask_Small;
  mitk::Image::Pointer m_IBSI_Phantom_Mask_Large;

public:

  void setUp(void) override
  {
    m_IBSI_Phantom_Image_Small = mitk::IOUtil::LoadImage(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Small.nrrd"));
    m_IBSI_Phantom_Image_Large = mitk::IOUtil::LoadImage(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Large.nrrd"));
    m_IBSI_Phantom_Mask_Small = mitk::IOUtil::LoadImage(GetTestDataFilePath("Radiomics/IBSI_Phantom_Mask_Small.nrrd"));
    m_IBSI_Phantom_Mask_Large = mitk::IOUtil::LoadImage(GetTestDataFilePath("Radiomics/IBSI_Phantom_Mask_Large.nrrd"));
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
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Zone Distance Variance with Large IBSI Phantom Image", 0, results["Grey Level Distance Zone::Zone Distance Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Grey Level Distance Zone::Zone Distance Entropy with Large IBSI Phantom Image", 1.92, results["Grey Level Distance Zone::Zone::Zone Distance Entropy"], 0.01);
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 144 features.", std::size_t(144), featureList.size());

    // These values are obtained with IBSI
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Low Dependence Emphasis with Large IBSI Phantom Image", 0.158, results["SliceWise Mean Neighbouring Grey Level Dependence::Low Dependence Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::High Dependence Emphasis with Large IBSI Phantom Image", 19.2, results["SliceWise Mean Neighbouring Grey Level Dependence::High Dependence Emphasis"], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Low Grey Level Count Emphasis with Large IBSI Phantom Image", 0.702, results["SliceWise Mean Neighbouring Grey Level Dependence::Low Grey Level Count Emphasis"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::High Grey Level Count Emphasis with Large IBSI Phantom Image", 7.49, results["SliceWise Mean Neighbouring Grey Level Dependence::High Grey Level Count Emphasis"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Low Dependence Low Grey Level Emphasis with Large IBSI Phantom Image", 0.0473, results["SliceWise Mean Neighbouring Grey Level Dependence::Low Dependence Low Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Low Dependence High Grey Level Emphasis with Large IBSI Phantom Image", 3.06, results["SliceWise Mean Neighbouring Grey Level Dependence::Low Dependence High Grey Level Emphasis"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::High Dependence Low Grey Level Emphasis with Large IBSI Phantom Image", 17.6, results["SliceWise Mean Neighbouring Grey Level Dependence::High Dependence Low Grey Level Emphasis"], 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::High Dependence High Grey Level Emphasis with Large IBSI Phantom Image", 49.5, results["SliceWise Mean Neighbouring Grey Level Dependence::High Dependence High Grey Level Emphasis"], 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Grey Level Non-Uniformity with Large IBSI Phantom Image", 10.2, results["SliceWise Mean Neighbouring Grey Level Dependence::Grey Level Non-Uniformity"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Grey Level Non-Uniformity Normalised with Large IBSI Phantom Image", 0.562, results["SliceWise Mean Neighbouring Grey Level Dependence::Grey Level Non-Uniformity Normalised"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Non-Uniformity with Large IBSI Phantom Image", 3.96, results["SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Non-Uniformity"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Non-Uniformity Normalised with Large IBSI Phantom Image", 0.212, results["SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Non-Uniformity Normalised"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Percentage with Large IBSI Phantom Image", 1, results["SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Percentage"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Grey Level Variance with Large IBSI Phantom Image", 2.7, results["SliceWise Mean Neighbouring Grey Level Dependence::Grey Level Variance"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Variance with Large IBSI Phantom Image", 2.73, results["SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Variance"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Entropy with Large IBSI Phantom Image", 2.71, results["SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Entropy"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Energy with Large IBSI Phantom Image", 0.17, results["SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Energy"], 0.01);

    // These values are obtained by manually running the tool
    // Values might be wrong.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Grey Level Mean with Large IBSI Phantom Image", 2.12, results["SliceWise Mean Neighbouring Grey Level Dependence::Grey Level Mean"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Mean with Large IBSI Phantom Image", 3.98, results["SliceWise Mean Neighbouring Grey Level Dependence::Dependence Count Mean"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Expected Neighbourhood Size with Large IBSI Phantom Image", 8, results["SliceWise Mean Neighbouring Grey Level Dependence::Expected Neighbourhood Size"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Average Neighbourhood Size with Large IBSI Phantom Image", 5.20, results["SliceWise Mean Neighbouring Grey Level Dependence::Average Neighbourhood Size"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Average Incomplete Neighbourhood Size with Large IBSI Phantom Image", 4.5598, results["SliceWise Mean Neighbouring Grey Level Dependence::Average Incomplete Neighbourhood Size"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Percentage of complete Neighbourhoods with Large IBSI Phantom Image", 0.1831, results["SliceWise Mean Neighbouring Grey Level Dependence::Percentage of complete Neighbourhoods"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbouring Grey Level Dependence::Percentage of Dependence Neighbour Voxels with Large IBSI Phantom Image", 0.579, results["SliceWise Mean Neighbouring Grey Level Dependence::Percentage of Dependence Neighbour Voxels"], 0.01);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFGreyLevelDistanceZone )
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

#include <mitkGIFNeighbouringGreyLevelDependenceFeatures.h>

class mitkGIFNeighbouringGreyLevelDependenceFeatureTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFNeighbouringGreyLevelDependenceFeatureTestSuite );

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
    mitk::GIFNeighbouringGreyLevelDependenceFeature::Pointer featureCalculator = mitk::GIFNeighbouringGreyLevelDependenceFeature::New();

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 24 features.", std::size_t(24), featureList.size());

    // These values are obtained with IBSI
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Low Dependence Emphasis with Large IBSI Phantom Image", 0.045, results["Neighbouring Grey Level Dependence::Low Dependence Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::High Dependence Emphasis with Large IBSI Phantom Image", 109, results["Neighbouring Grey Level Dependence::High Dependence Emphasis"], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Low Grey Level Count Emphasis with Large IBSI Phantom Image", 0.693, results["Neighbouring Grey Level Dependence::Low Grey Level Count Emphasis"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::High Grey Level Count Emphasis with Large IBSI Phantom Image", 7.66, results["Neighbouring Grey Level Dependence::High Grey Level Count Emphasis"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Low Dependence Low Grey Level Emphasis with Large IBSI Phantom Image", 0.00963, results["Neighbouring Grey Level Dependence::Low Dependence Low Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Low Dependence High Grey Level Emphasis with Large IBSI Phantom Image", 0.736, results["Neighbouring Grey Level Dependence::Low Dependence High Grey Level Emphasis"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::High Dependence Low Grey Level Emphasis with Large IBSI Phantom Image", 102, results["Neighbouring Grey Level Dependence::High Dependence Low Grey Level Emphasis"], 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::High Dependence High Grey Level Emphasis with Large IBSI Phantom Image", 235, results["Neighbouring Grey Level Dependence::High Dependence High Grey Level Emphasis"], 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Grey Level Non-Uniformity with Large IBSI Phantom Image", 37.9, results["Neighbouring Grey Level Dependence::Grey Level Non-Uniformity"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Grey Level Non-Uniformity Normalised with Large IBSI Phantom Image", 0.512, results["Neighbouring Grey Level Dependence::Grey Level Non-Uniformity Normalised"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Dependence Count Non-Uniformity with Large IBSI Phantom Image", 4.86,              results["Neighbouring Grey Level Dependence::Dependence Count Non-Uniformity"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Dependence Count Non-Uniformity Normalised with Large IBSI Phantom Image", 0.0657, results["Neighbouring Grey Level Dependence::Dependence Count Non-Uniformity Normalised"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Dependence Count Percentage with Large IBSI Phantom Image", 1, results["Neighbouring Grey Level Dependence::Dependence Count Percentage"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Grey Level Variance with Large IBSI Phantom Image", 3.05, results["Neighbouring Grey Level Dependence::Grey Level Variance"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Dependence Count Variance with Large IBSI Phantom Image", 22.1, results["Neighbouring Grey Level Dependence::Dependence Count Variance"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Dependence Count Entropy with Large IBSI Phantom Image", 4.4, results["Neighbouring Grey Level Dependence::Dependence Count Entropy"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Dependence Count Energy with Large IBSI Phantom Image", 0.0533, results["Neighbouring Grey Level Dependence::Dependence Count Energy"], 0.01);

    // These values are obtained by manually running the tool
    // Values might be wrong.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Grey Level Mean with Large IBSI Phantom Image", 2.15, results["Neighbouring Grey Level Dependence::Grey Level Mean"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Dependence Count Mean with Large IBSI Phantom Image", 9.32, results["Neighbouring Grey Level Dependence::Dependence Count Mean"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Expected Neighbourhood Size with Large IBSI Phantom Image", 26, results["Neighbouring Grey Level Dependence::Expected Neighbourhood Size"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Average Neighbourhood Size with Large IBSI Phantom Image", 14.24, results["Neighbouring Grey Level Dependence::Average Neighbourhood Size"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Average Incomplete Neighbourhood Size with Large IBSI Phantom Image", 14.24, results["Neighbouring Grey Level Dependence::Average Incomplete Neighbourhood Size"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Percentage of complete Neighbourhoods with Large IBSI Phantom Image", 0, results["Neighbouring Grey Level Dependence::Percentage of complete Neighbourhoods"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbouring Grey Level Dependence::Percentage of Dependence Neighbour Voxels with Large IBSI Phantom Image", 0.584, results["Neighbouring Grey Level Dependence::Percentage of Dependence Neighbour Voxels"], 0.01);
   }

  void ImageDescription_PhantomTest_2D()
  {
    mitk::GIFNeighbouringGreyLevelDependenceFeature::Pointer featureCalculator = mitk::GIFNeighbouringGreyLevelDependenceFeature::New();

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

MITK_TEST_SUITE_REGISTRATION(mitkGIFNeighbouringGreyLevelDependenceFeature )
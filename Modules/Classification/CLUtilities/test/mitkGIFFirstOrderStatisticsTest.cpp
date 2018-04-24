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

#include <mitkGIFFirstOrderStatistics.h>

class mitkGIFFirstOrderStatisticsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFFirstOrderStatisticsTestSuite);

  MITK_TEST(ImageDescription_PhantomTest);

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

  void ImageDescription_PhantomTest()
  {
    mitk::GIFFirstOrderStatistics::Pointer featureCalculator = mitk::GIFFirstOrderStatistics::New();

    featureCalculator->SetUseBinsize(true);
    featureCalculator->SetBinsize(1);
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 50 features.", std::size_t(50), featureList.size());

    // These values are obtained by a run of the filter.
    // The might be wrong!


    // These values are taken from the IBSI Initiative to ensure compatibility
    // The values are given with an accuracy of 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Mean with Large IBSI Phantom Image", 2.15, results["First Order::Mean"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Unbiased Variance with Large IBSI Phantom Image", 3.09, results["First Order::Unbiased Variance"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Biased Variance with Large IBSI Phantom Image", 3.05, results["First Order::Biased Variance"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Skewness with Large IBSI Phantom Image", 1.08, results["First Order::Skewness"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Kurtosis with Large IBSI Phantom Image", 2.65, results["First Order::Kurtosis"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Median with Large IBSI Phantom Image", 1, results["First Order::Median"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Minimum with Large IBSI Phantom Image", 1, results["First Order::Minimum"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Maximum with Large IBSI Phantom Image", 6, results["First Order::Maximum"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Range with Large IBSI Phantom Image", 5, results["First Order::Range"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Mean Absolute Deviation with Large IBSI Phantom Image", 1.55, results["First Order::Mean Absolute Deviation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Robust Mean Absolute Deviation with Large IBSI Phantom Image", 1.11, results["First Order::Robust Mean Absolute Deviation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Median Absolute Deviation with Large IBSI Phantom Image", 1.15, results["First Order::Median Absolute Deviation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Coefficient of variation with Large IBSI Phantom Image", 0.812, results["First Order::Coefficient Of Variation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Quantile coefficient of dispersion with Large IBSI Phantom Image", 0.625, results["First Order::Quantile Coefficient Of Dispersion"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Energy with Large IBSI Phantom Image", 567, results["First Order::Energy"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Root mean square with Large IBSI Phantom Image", 2.77, results["First Order::Root Mean Square"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Entropy with Large IBSI Phantom Image", 1.26561, results["First Order::Entropy"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Excess Kurtosis with Large IBSI Phantom Image", -0.35462, results["First Order::Excess Kurtosis"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Covered image intensity range with Large IBSI Phantom Image", 0.5555555, results["First Order::Covered Image Intensity Range"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Sum of voxels with Large IBSI Phantom Image", 159, results["First Order::Sum"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Mode with Large IBSI Phantom Image", 1, results["First Order::Mode"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Mode Probability with Large IBSI Phantom Image", 50, results["First Order::Mode Probability"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Unbiased Standard deviation with Large IBSI Phantom Image", 1.757, results["First Order::Unbiased Standard deviation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Biased Standard deviation with Large IBSI Phantom Image", 1.74627, results["First Order::Biased Standard deviation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Robust mean with Large IBSI Phantom Image", 1.74627, results["First Order::Robust Mean"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Number Of Voxels with Large IBSI Phantom Image", 74, results["First Order::Number Of Voxels"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::10th Percentile with Large IBSI Phantom Image", 0.648, results["First Order::10th Percentile"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::90th Percentile with Large IBSI Phantom Image", 4.475, results["First Order::90th Percentile"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Interquantile Range with Large IBSI Phantom Image", 2.91125, results["First Order::Interquartile Range"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Voxel Space with Large IBSI Phantom Image", 8, results["First Order::Voxel Space"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Voxel Volume with Large IBSI Phantom Image", 8, results["First Order::Voxel Volume"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order::Image Dimension with Large IBSI Phantom Image", 3, results["First Order::Image Dimension"], 0.01);

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFFirstOrderStatistics )
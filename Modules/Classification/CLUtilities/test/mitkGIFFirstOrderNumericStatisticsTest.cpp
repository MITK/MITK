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

#include <mitkGIFFirstOrderNumericStatistics.h>

class mitkGIFFirstOrderNumericStatisticsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFFirstOrderNumericStatisticsTestSuite);

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
    mitk::GIFFirstOrderNumericStatistics::Pointer featureCalculator = mitk::GIFFirstOrderNumericStatistics::New();

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 50 features.", std::size_t(50), featureList.size());

    // These values are obtained by a run of the filter.
    // The might be wrong!
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Mean with Large IBSI Phantom Image", 2.15, results["First Order Numeric::Mean"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Variance with Large IBSI Phantom Image", 3.05, results["First Order Numeric::Variance"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Skewness with Large IBSI Phantom Image", 1.08, results["First Order Numeric::Skewness"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Excess kurtosis with Large IBSI Phantom Image", -0.355, results["First Order Numeric::Excess kurtosis"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Median with Large IBSI Phantom Image", 1, results["First Order Numeric::Median"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Minimum with Large IBSI Phantom Image", 1, results["First Order Numeric::Minimum"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::10th Percentile with Large IBSI Phantom Image", 1, results["First Order Numeric::10th Percentile"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::90th Percentile with Large IBSI Phantom Image", 4, results["First Order Numeric::90th Percentile"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Maximum with Large IBSI Phantom Image", 6, results["First Order Numeric::Maximum"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Interquantile range with Large IBSI Phantom Image", 3, results["First Order Numeric::Interquantile range"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Range with Large IBSI Phantom Image", 5, results["First Order Numeric::Range"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Mean absolute deviation with Large IBSI Phantom Image", 1.55, results["First Order Numeric::Mean absolute deviation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Robust mean absolute deviation with Large IBSI Phantom Image", 1.11, results["First Order Numeric::Robust mean absolute deviation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Median absolute deviation with Large IBSI Phantom Image", 1.15, results["First Order Numeric::Median absolute deviation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Coefficient of variation with Large IBSI Phantom Image", 0.812, results["First Order Numeric::Coefficient of variation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Quantile coefficient of dispersion with Large IBSI Phantom Image", 0.6, results["First Order Numeric::Quantile coefficient of dispersion"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Energy with Large IBSI Phantom Image", 567, results["First Order Numeric::Energy"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Root mean square with Large IBSI Phantom Image", 2.77, results["First Order Numeric::Root mean square"], 0.01);


    // These values are taken from the IBSI Initiative to ensure compatibility
    // The values are given with an accuracy of 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Standard Deviation with Large IBSI Phantom Image", 1.74513, results["First Order Numeric::Standard Deviation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Kurtosis with Large IBSI Phantom Image", 2.64538, results["First Order Numeric::Kurtosis"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Robust mean with Large IBSI Phantom Image", 1.74627, results["First Order Numeric::Robust mean"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Robust variance with Large IBSI Phantom Image", 1.65204, results["First Order Numeric::Robust variance"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Covered image intensity range with Large IBSI Phantom Image", 0.83333, results["First Order Numeric::Covered image intensity range"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Mode index with Large IBSI Phantom Image",0 , results["First Order Numeric::Mode index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Mode value with Large IBSI Phantom Image", 1, results["First Order Numeric::Mode value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Mode probability with Large IBSI Phantom Image", 0.675676, results["First Order Numeric::Mode probability"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Entropy with Large IBSI Phantom Image", -1.26561, results["First Order Numeric::Entropy"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Uniformtiy with Large IBSI Phantom Image", 0.512418, results["First Order Numeric::Uniformtiy"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Number of voxels with Large IBSI Phantom Image", 74 , results["First Order Numeric::Number of voxels"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Sum of voxels with Large IBSI Phantom Image", 159, results["First Order Numeric::Sum of voxels"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Voxel space with Large IBSI Phantom Image", 8, results["First Order Numeric::Voxel space"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Voxel volume with Large IBSI Phantom Image", 8, results["First Order Numeric::Voxel volume"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Numeric::Image Dimension with Large IBSI Phantom Image", 3, results["First Order Numeric::Image Dimension"], 0.01);

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFFirstOrderNumericStatistics )
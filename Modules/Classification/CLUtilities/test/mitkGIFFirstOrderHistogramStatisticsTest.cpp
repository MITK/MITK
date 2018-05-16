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

#include <mitkGIFFirstOrderHistogramStatistics.h>

class mitkGIFFirstOrderHistogramStatisticsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFFirstOrderHistogramStatisticsTestSuite);

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
    mitk::GIFFirstOrderHistogramStatistics::Pointer featureCalculator = mitk::GIFFirstOrderHistogramStatistics::New();

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 46 features.", std::size_t(46), featureList.size());

    // These values are obtained by a run of the filter.
    // The might be wrong!
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Mean Value should be 2.15 with Large IBSI Phantom Image", 2.15, results["First Order Histogram::Mean Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Variance Value should be 3.05 with Large IBSI Phantom Image", 3.05, results["First Order Histogram::Variance Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Skewness Value should be 1.08 with Large IBSI Phantom Image", 1.08, results["First Order Histogram::Skewness Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Excess Kurtosis Value should be -0.355 with Large IBSI Phantom Image", -0.355, results["First Order Histogram::Excess Kurtosis Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Median Value should be 1 with Large IBSI Phantom Image", 1.0, results["First Order Histogram::Median Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Minimum Value should be 1 with Large IBSI Phantom Image", 1, results["First Order Histogram::Minimum Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Percentile 10 Value should be 0.648 with Large IBSI Phantom Image", 0.648, results["First Order Histogram::Percentile 10 Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Percentile 90 Value should be 4.475 with Large IBSI Phantom Image", 4.475, results["First Order Histogram::Percentile 90 Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Maximum Value should be 6 with Large IBSI Phantom Image", 6, results["First Order Histogram::Maximum Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Mode Value should be 1 with Large IBSI Phantom Image", 1, results["First Order Histogram::Mode Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Interquantile Range Value should be 2.9 with Large IBSI Phantom Image", 2.911, results["First Order Histogram::Interquantile Range Value"], 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Range Value should be 5 with Large IBSI Phantom Image", 5, results["First Order Histogram::Range Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Mean Absolute Deviation Value should be 1.55 with Large IBSI Phantom Image", 1.55, results["First Order Histogram::Mean Absolute Deviation Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Robust Mean Absolute Deviation Value should be 1.11 with Large IBSI Phantom Image", 1.11, results["First Order Histogram::Robust Mean Absolute Deviation Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Median Absolute Deviation Value should be 1.14 with Large IBSI Phantom Image", 1.14, results["First Order Histogram::Median Absolute Deviation Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Coefficient of Variation Value should be 0.812 with Large IBSI Phantom Image", 0.812, results["First Order Histogram::Coefficient of Variation Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Quantile coefficient of Dispersion Value should be 0.626 with Large IBSI Phantom Image", 0.626, results["First Order Histogram::Quantile coefficient of Dispersion Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Entropy Value should be 1.27 with Large IBSI Phantom Image", 1.27, results["First Order Histogram::Entropy Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Uniformity Value should be 0.512 with Large IBSI Phantom Image", 0.512, results["First Order Histogram::Uniformity Value"], 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Robust Mean Index should be 0.746 with Large IBSI Phantom Image", 0.746, results["First Order Histogram::Robust Mean Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Robust Mean Value should be 1.746 with Large IBSI Phantom Image", 1.746, results["First Order Histogram::Robust Mean Value"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Number of Bins should be 6", 6, results["First Order Histogram::Number of Bins"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Bin Size should be 1", 1, results["First Order Histogram::Bin Size"], 0.01);

    // These values are taken from the IBSI Initiative to ensure compatibility
    // The values are given with an accuracy of 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Mean Index should be 2.15 with Large IBSI Phantom Image", 2.15, results["First Order Histogram::Mean Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Variance Index should be 3.05 with Large IBSI Phantom Image", 3.05, results["First Order Histogram::Variance Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Skewness Index should be 1.08 with Large IBSI Phantom Image", 1.08, results["First Order Histogram::Skewness Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Excess Kurtosis Index should be -0.355 with Large IBSI Phantom Image", -0.355, results["First Order Histogram::Excess Kurtosis Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Median Index should be 1 with Large IBSI Phantom Image", 1.0, results["First Order Histogram::Median Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Minimum Index should be 1 with Large IBSI Phantom Image", 1, results["First Order Histogram::Minimum Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Percentile 10 Index should be 1 with Large IBSI Phantom Image", 1, results["First Order Histogram::Percentile 10 Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Percentile 90 Index should be 2.15 with Large IBSI Phantom Image", 4, results["First Order Histogram::Percentile 90 Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Maximum Index should be 6 with Large IBSI Phantom Image", 6, results["First Order Histogram::Maximum Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Mode Index should be 1 with Large IBSI Phantom Image", 1, results["First Order Histogram::Mode Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Interquantile Range Index should be 3 with Large IBSI Phantom Image", 3, results["First Order Histogram::Interquantile Range Index"], 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Range Index should be 5 with Large IBSI Phantom Image", 5, results["First Order Histogram::Range Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Mean Absolute Deviation Index should be 3 with Large IBSI Phantom Image", 1.55, results["First Order Histogram::Mean Absolute Deviation Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Robust Mean Absolute Deviation Index should be 1.11 with Large IBSI Phantom Image", 1.11, results["First Order Histogram::Robust Mean Absolute Deviation Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Median Absolute Deviation Index should be 1.14 with Large IBSI Phantom Image", 1.14, results["First Order Histogram::Median Absolute Deviation Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Coefficient of Variation Index should be 0.812 with Large IBSI Phantom Image", 0.812, results["First Order Histogram::Coefficient of Variation Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Quantile coefficient of Dispersion Index should be 0.6 with Large IBSI Phantom Image", 0.6, results["First Order Histogram::Quantile coefficient of Dispersion Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Entropy Index should be 1.27 with Large IBSI Phantom Image", 1.27, results["First Order Histogram::Entropy Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Uniformity Index should be 0.512 with Large IBSI Phantom Image", 0.512, results["First Order Histogram::Uniformity Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Maximum Gradient should be 8 with Large IBSI Phantom Image", 8, results["First Order Histogram::Maximum Gradient"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Maximum Gradient Index should be 3 with Large IBSI Phantom Image", 3, results["First Order Histogram::Maximum Gradient Index"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Minimum Gradient should be -50 with Large IBSI Phantom Image", -50, results["First Order Histogram::Minimum Gradient"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("First Order Histogram::Minimum Gradient Index should be 3 with Large IBSI Phantom Image", 1, results["First Order Histogram::Minimum Gradient Index"], 0.01);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFFirstOrderHistogramStatistics )
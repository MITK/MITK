/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include "mitkIOUtil.h"
#include <cmath>

#include <mitkGIFCurvatureStatistic.h>

class mitkGIFCurvatureStatisticTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFCurvatureStatisticTestSuite);

  MITK_TEST(ImageDescription_PhantomTest);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_IBSI_Phantom_Image_Large;
  mitk::Image::Pointer m_IBSI_Phantom_Mask_Large;

public:

  void setUp(void) override
  {
    m_IBSI_Phantom_Image_Large = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Large.nrrd"));
    m_IBSI_Phantom_Mask_Large = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Mask_Large.nrrd"));
  }

  void ImageDescription_PhantomTest()
  {
    mitk::GIFCurvatureStatistic::Pointer featureCalculator = mitk::GIFCurvatureStatistic::New();

    featureCalculator->SetUseBinsize(true);
    featureCalculator->SetBinsize(1.0);
    featureCalculator->SetUseMinimumIntensity(true);
    featureCalculator->SetUseMaximumIntensity(true);
    featureCalculator->SetMinimumIntensity(0.5);
    featureCalculator->SetMaximumIntensity(6.5);

    auto featureList = featureCalculator->CalculateFeatures(m_IBSI_Phantom_Image_Large, m_IBSI_Phantom_Mask_Large);

    std::map<std::string, double> results;
    for (const auto &valuePair : featureList)
    {
      MITK_INFO << mitk::AbstractGlobalImageFeature::GenerateLegacyFeatureNameWOEncoding(valuePair.first) << " : " << valuePair.second;
      results[mitk::AbstractGlobalImageFeature::GenerateLegacyFeatureNameWOEncoding(valuePair.first)] = valuePair.second;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 44 features.", std::size_t(44), featureList.size());

    // These values are obtained by a run of the filter.
    // The might be wrong!
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Minimum Mean Curvature with Large IBSI Phantom Image", -1.51, results["Curvature Feature::Minimum Mean Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Maximum Mean Curvature with Large IBSI Phantom Image", 0.51, results["Curvature Feature::Maximum Mean Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Mean Curvature with Large IBSI Phantom Image", 0.095, results["Curvature Feature::Mean Mean Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Mean Curvature with Large IBSI Phantom Image", 0.45, results["Curvature Feature::Standard Deviation Mean Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Mean Curvature with Large IBSI Phantom Image", -2.55, results["Curvature Feature::Skewness Mean Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Positive Mean Curvature with Large IBSI Phantom Image", 0.30, results["Curvature Feature::Mean Positive Mean Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Positive Mean Curvature with Large IBSI Phantom Image", 0.12, results["Curvature Feature::Standard Deviation Positive Mean Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Positive Mean Curvature with Large IBSI Phantom Image", 0.60, results["Curvature Feature::Skewness Positive Mean Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Negative Mean Curvature with Large IBSI Phantom Image", -0.955, results["Curvature Feature::Mean Negative Mean Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Negative Mean Curvature with Large IBSI Phantom Image", 0.56, results["Curvature Feature::Standard Deviation Negative Mean Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Negative Mean Curvature with Large IBSI Phantom Image", 0.09, results["Curvature Feature::Skewness Negative Mean Curvature"], 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Minimum Gaussian Curvature with Large IBSI Phantom Image", -0.211, results["Curvature Feature::Minimum Gaussian Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Maximum Gaussian Curvature with Large IBSI Phantom Image", 1.81, results["Curvature Feature::Maximum Gaussian Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Gaussian Curvature with Large IBSI Phantom Image", 0.14, results["Curvature Feature::Mean Gaussian Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Gaussian Curvature with Large IBSI Phantom Image", 0.42, results["Curvature Feature::Standard Deviation Gaussian Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Gaussian Curvature with Large IBSI Phantom Image", 3.51, results["Curvature Feature::Skewness Gaussian Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Positive Gaussian Curvature with Large IBSI Phantom Image", 0.38, results["Curvature Feature::Mean Positive Gaussian Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Positive Gaussian Curvature with Large IBSI Phantom Image", 0.60, results["Curvature Feature::Standard Deviation Positive Gaussian Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Positive Gaussian Curvature with Large IBSI Phantom Image", 1.89, results["Curvature Feature::Skewness Positive Gaussian Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Negative Gaussian Curvature with Large IBSI Phantom Image", -0.11, results["Curvature Feature::Mean Negative Gaussian Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Negative Gaussian Curvature with Large IBSI Phantom Image", 0.049, results["Curvature Feature::Standard Deviation Negative Gaussian Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Negative Gaussian Curvature with Large IBSI Phantom Image", -0.74, results["Curvature Feature::Skewness Negative Gaussian Curvature"], 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Minimum Minimum Curvature with Large IBSI Phantom Image", -2.19, results["Curvature Feature::Minimum Minimum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Maximum Minimum Curvature with Large IBSI Phantom Image", 0.35, results["Curvature Feature::Maximum Minimum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Minimum Curvature with Large IBSI Phantom Image", -0.11, results["Curvature Feature::Mean Minimum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Minimum Curvature with Large IBSI Phantom Image", 0.573, results["Curvature Feature::Standard Deviation Minimum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Minimum Curvature with Large IBSI Phantom Image", -2.742, results["Curvature Feature::Skewness Minimum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Positive Minimum Curvature with Large IBSI Phantom Image", 0.264, results["Curvature Feature::Mean Positive Minimum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Positive Minimum Curvature with Large IBSI Phantom Image", 0.132, results["Curvature Feature::Standard Deviation Positive Minimum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Positive Minimum Curvature with Large IBSI Phantom Image", -1.335, results["Curvature Feature::Skewness Positive Minimum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Negative Minimum Curvature with Large IBSI Phantom Image", -0.946, results["Curvature Feature::Mean Negative Minimum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Negative Minimum Curvature with Large IBSI Phantom Image", 0.844, results["Curvature Feature::Standard Deviation Negative Minimum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Negative Minimum Curvature with Large IBSI Phantom Image", -0.599, results["Curvature Feature::Skewness Negative Minimum Curvature"], 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Minimum Maximum Curvature with Large IBSI Phantom Image", -0.83, results["Curvature Feature::Minimum Maximum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Maximum Maximum Curvature with Large IBSI Phantom Image", 0.79, results["Curvature Feature::Maximum Maximum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Maximum Curvature with Large IBSI Phantom Image", 0.30, results["Curvature Feature::Mean Maximum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Maximum Curvature with Large IBSI Phantom Image", 0.369, results["Curvature Feature::Standard Deviation Maximum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Maximum Curvature with Large IBSI Phantom Image", -1.617, results["Curvature Feature::Skewness Maximum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Positive Maximum Curvature with Large IBSI Phantom Image", 0.513, results["Curvature Feature::Mean Positive Maximum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Positive Maximum Curvature with Large IBSI Phantom Image", 0.094, results["Curvature Feature::Standard Deviation Positive Maximum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Positive Maximum Curvature with Large IBSI Phantom Image", 1.548, results["Curvature Feature::Skewness Positive Maximum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Mean Negative Maximum Curvature with Large IBSI Phantom Image", -0.47, results["Curvature Feature::Mean Negative Maximum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Standard Deviation Negative Maximum Curvature with Large IBSI Phantom Image", 0.394, results["Curvature Feature::Standard Deviation Negative Maximum Curvature"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Negative Maximum Curvature with Large IBSI Phantom Image", 0.273, results["Curvature Feature::Skewness Negative Maximum Curvature"], 0.01);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFCurvatureStatistic )

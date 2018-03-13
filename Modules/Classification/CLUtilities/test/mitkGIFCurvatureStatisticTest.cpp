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

#include <mitkGIFCurvatureStatistic.h>

class mitkGIFCurvatureStatisticTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFCurvatureStatisticTestSuite);

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
    m_IBSI_Phantom_Image_Small = mitk::IOUtil::LoadImage(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Small.nrrd"));
    m_IBSI_Phantom_Image_Large = mitk::IOUtil::LoadImage(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Large.nrrd"));
    m_IBSI_Phantom_Mask_Small = mitk::IOUtil::LoadImage(GetTestDataFilePath("Radiomics/IBSI_Phantom_Mask_Small.nrrd"));
    m_IBSI_Phantom_Mask_Large = mitk::IOUtil::LoadImage(GetTestDataFilePath("Radiomics/IBSI_Phantom_Mask_Large.nrrd"));
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
    for (auto valuePair : featureList)
    {
      MITK_INFO << valuePair.first << " : " << valuePair.second;
      results[valuePair.first] = valuePair.second;
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
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Curvature Feature::Skewness Positive Mean Curvature with Large IBSI Phantom Image", 1195, results["Curvature Feature::Skewness Positive Mean Curvature"], 0.01);
   }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFCurvatureStatistic )
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

#include <mitkGIFVolumetricDensityStatistics.h>

class mitkGIFVolumetricDensityStatisticsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFVolumetricDensityStatisticsTestSuite);

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
    mitk::GIFVolumetricDensityStatistics::Pointer featureCalculator = mitk::GIFVolumetricDensityStatistics::New();

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 13 features.", std::size_t(13), featureList.size());

    // These values are obtained by a run of the filter.
    // The might be wrong!

    // These values are obtained in collaboration with IBSI.
    // They are usually reported with an accuracy of 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Volume integrated intensity with Large IBSI Phantom Image", 1195, results["Morphological Density::Volume integrated intensity"], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Volume Moran's I index with Large IBSI Phantom Image", 0.0397, results["Morphological Density::Volume Moran's I index"], 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Volume Geary's C measure with Large IBSI Phantom Image", 0.974, results["Morphological Density::Volume Geary's C measure"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Volume Volume density axis-aligned bounding box with Large IBSI Phantom Image", 0.87, results["Morphological Density::Volume density axis-aligned bounding box"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Surface Volume density axis-aligned bounding box with Large IBSI Phantom Image", 0.87, results["Morphological Density::Surface density axis-aligned bounding box"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Volume Volume oriented minimum bounding box with Large IBSI Phantom Image", 0.87, results["Morphological Density::Volume density oriented minimum bounding box"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Surface Volume oriented minimum bounding box with Large IBSI Phantom Image", 0.86, results["Morphological Density::Surface density oriented minimum bounding box"], 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Volume Volume approx. enclosing ellipsoid with Large IBSI Phantom Image", 1.17, results["Morphological Density::Volume density approx. enclosing ellipsoid"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Surface Volume approx. enclosing ellipsoid with Large IBSI Phantom Image", 1.34, results["Morphological Density::Surface density approx. enclosing ellipsoid"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Volume Volume minimum volume enclosing ellipsoid with Large IBSI Phantom Image", 0.24, results["Morphological Density::Volume density approx. minimum volume enclosing ellipsoid"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Surface Volume minimum volume enclosing ellipsoid with Large IBSI Phantom Image", 0.49, results["Morphological Density::Surface density approx. minimum volume enclosing ellipsoid"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Volume Volume convex hull with Large IBSI Phantom Image", 0.96, results["Morphological Density::Volume density convex hull"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Morphological Density::Surface Volume convex hull with Large IBSI Phantom Image", 1.03, results["Morphological Density::Surface density convex hull"], 0.01);
   }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFVolumetricDensityStatistics )
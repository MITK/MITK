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

#include <mitkGIFVolumetricStatistics.h>

class mitkGIFVolumetricStatisticsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFVolumetricStatisticsTestSuite);

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
    mitk::GIFVolumetricStatistics::Pointer featureCalculator = mitk::GIFVolumetricStatistics::New();

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Volume Statistic should calculate 33 features.", std::size_t(33), featureList.size());

    // These values are obtained in cooperation with IBSI
    // Default accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Volume (mesh based) with Large IBSI Phantom Image", 556, results["Volumetric Features::Volume (mesh based)"], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Volume (voxel based) with Large IBSI Phantom Image", 592, results["Volumetric Features::Volume (voxel based)"], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Surface (mesh based) with Large IBSI Phantom Image", 388, results["Volumetric Features::Surface (mesh based)"], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Surface to volume ratio (mesh based) with Large IBSI Phantom Image", 0.656, results["Volumetric Features::Surface to volume ratio (mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Compactness 1 (mesh based) with Large IBSI Phantom Image", 0.0437, results["Volumetric Features::Compactness 1 (mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Compactness 2 (mesh based) with Large IBSI Phantom Image", 0.678, results["Volumetric Features::Compactness 2 (mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features:: with Large IBSI Phantom Image", 1.51, results["Volumetric Features::"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features:: with Large IBSI Phantom Image", 1.51, results["Volumetric Features::"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features:: with Large IBSI Phantom Image", 1.51, results["Volumetric Features::"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features:: with Large IBSI Phantom Image", 1.51, results["Volumetric Features::"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features:: with Large IBSI Phantom Image", 1.51, results["Volumetric Features::"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features:: with Large IBSI Phantom Image", 1.51, results["Volumetric Features::"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features:: with Large IBSI Phantom Image", 1.51, results["Volumetric Features::"], 0.01);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFVolumetricStatistics )
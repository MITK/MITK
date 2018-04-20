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

#include <mitkGIFIntensityVolumeHistogramFeatures.h>

class mitkGIFIntensityVolumeHistogramTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE( mitkGIFIntensityVolumeHistogramTestSuite);

  MITK_TEST(ImageDescription_PhantomTest_Small);

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

  void ImageDescription_PhantomTest_Small()
  {
    mitk::GIFIntensityVolumeHistogramFeatures::Pointer featureCalculator = mitk::GIFIntensityVolumeHistogramFeatures::New();

    auto featureList = featureCalculator->CalculateFeatures(m_IBSI_Phantom_Image_Small, m_IBSI_Phantom_Mask_Small);

    std::map<std::string, double> results;
    for (auto valuePair : featureList)
    {
      MITK_INFO << valuePair.first << " : " << valuePair.second;
      results[valuePair.first] = valuePair.second;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 7 features.", std::size_t(2), featureList.size());

    // These values are obtained in cooperation with IBSI
    // Reported with an accuracy of 0.1
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Local Intensity::Local Intensity Peak with Large IBSI Phantom Image", 2.6, results["Local Intensity::Local Intensity Peak"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Local Intensity::Global Intensity Peak with Large IBSI Phantom Image", 3.1, results["Local Intensity::Global Intensity Peak"], 0.1);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFIntensityVolumeHistogram)
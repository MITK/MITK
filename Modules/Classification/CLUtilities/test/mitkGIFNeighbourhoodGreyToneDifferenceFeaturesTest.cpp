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

#include <mitkGIFNeighbourhoodGreyToneDifferenceFeatures.h>

class mitkGIFNeighbourhoodGreyToneDifferenceFeaturesTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFNeighbourhoodGreyToneDifferenceFeaturesTestSuite);

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
    mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::Pointer featureCalculator = mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::New();

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 5 features.", std::size_t(5), featureList.size());

    // These values are obtained with IBSI
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbourhood Grey Tone Difference::Coarsness with Large IBSI Phantom Image", 0.0296, results["Neighbourhood Grey Tone Difference::Coarsness"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbourhood Grey Tone Difference::Contrast with Large IBSI Phantom Image", 0.584, results["Neighbourhood Grey Tone Difference::Contrast"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbourhood Grey Tone Difference::Busyness with Large IBSI Phantom Image", 6.54, results["Neighbourhood Grey Tone Difference::Busyness"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbourhood Grey Tone Difference::Complexity with Large IBSI Phantom Image", 13.5, results["Neighbourhood Grey Tone Difference::Complexity"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neighbourhood Grey Tone Difference::Strength with Large IBSI Phantom Image", 0.763, results["Neighbourhood Grey Tone Difference::Strength"], 0.01);
   }

  void ImageDescription_PhantomTest_2D()
  {
    mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::Pointer featureCalculator = mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::New();

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 30 features.", std::size_t(30), featureList.size());

    // These values are obtained with IBSI
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbourhood Grey Tone Difference::Coarsness with Large IBSI Phantom Image", 0.121, results["SliceWise Mean Neighbourhood Grey Tone Difference::Coarsness"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbourhood Grey Tone Difference::Contrast with Large IBSI Phantom Image", 0.925, results["SliceWise Mean Neighbourhood Grey Tone Difference::Contrast"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbourhood Grey Tone Difference::Busyness with Large IBSI Phantom Image", 2.99, results["SliceWise Mean Neighbourhood Grey Tone Difference::Busyness"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbourhood Grey Tone Difference::Complexity with Large IBSI Phantom Image", 10.4, results["SliceWise Mean Neighbourhood Grey Tone Difference::Complexity"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Neighbourhood Grey Tone Difference::Strength with Large IBSI Phantom Image", 2.88, results["SliceWise Mean Neighbourhood Grey Tone Difference::Strength"], 0.01);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFNeighbourhoodGreyToneDifferenceFeatures )
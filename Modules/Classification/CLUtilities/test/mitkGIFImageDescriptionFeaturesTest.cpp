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

#include <mitkGIFImageDescriptionFeatures.h>

class mitkGIFImageDescriptionFeaturesTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFImageDescriptionFeaturesTestSuite );

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
    mitk::GIFImageDescriptionFeatures::Pointer featureCalculator = mitk::GIFImageDescriptionFeatures::New();
    auto featureList = featureCalculator->CalculateFeatures(m_IBSI_Phantom_Image_Large, m_IBSI_Phantom_Mask_Large);

    std::map<std::string, double> results;
    for (auto valuePair : featureList)
    {
      MITK_INFO << valuePair.first << " : " << valuePair.second;
      results[valuePair.first] = valuePair.second;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 22 features.", std::size_t(22), featureList.size());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Diagnostic::Image Dimension X should be 7 with Large IBSI Phantom Image", int(7), int(results["Diagnostic::Image Dimension X"]));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Diagnostic::Image Dimension Y should be 6 with Large IBSI Phantom Image", int(6), int(results["Diagnostic::Image Dimension Y"]));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Diagnostic::Image Dimension Z should be 6 with Large IBSI Phantom Image", int(6), int(results["Diagnostic::Image Dimension Z"]));
    
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Diagnostic::Image Spacing X should be 2 with Large IBSI Phantom Image", 2.0, results["Diagnostic::Image Spacing X"], 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Diagnostic::Image Spacing Y should be 2 with Large IBSI Phantom Image", 2.0, results["Diagnostic::Image Spacing Y"], 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Diagnostic::Image Spacing Z should be 2 with Large IBSI Phantom Image", 2.0, results["Diagnostic::Image Spacing Z"], 0.0001);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFImageDescriptionFeatures )
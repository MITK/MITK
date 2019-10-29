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
    m_IBSI_Phantom_Image_Small = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Small.nrrd"));
    m_IBSI_Phantom_Image_Large = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Large.nrrd"));
    m_IBSI_Phantom_Mask_Small = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Mask_Small.nrrd"));
    m_IBSI_Phantom_Mask_Large = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Mask_Large.nrrd"));
  }

  void ImageDescription_PhantomTest()
  {
    mitk::GIFVolumetricStatistics::Pointer featureCalculator = mitk::GIFVolumetricStatistics::New();

    auto featureList = featureCalculator->CalculateFeatures(m_IBSI_Phantom_Image_Large, m_IBSI_Phantom_Mask_Large);

    std::map<std::string, double> results;
    for (auto valuePair : featureList)
    {
      MITK_INFO << valuePair.first << " : " << valuePair.second;
      results[valuePair.first] = valuePair.second;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Volume Statistic should calculate 33 features.", std::size_t(38), featureList.size());

    // These values are obtained in cooperation with IBSI
    // Default accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Volume (mesh based) with Large IBSI Phantom Image", 556, results["Volumetric Features::Volume (mesh based)"], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Volume (voxel based) with Large IBSI Phantom Image", 592, results["Volumetric Features::Volume (voxel based)"], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Surface (mesh based) with Large IBSI Phantom Image", 388, results["Volumetric Features::Surface (mesh based)"], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Surface to volume ratio (mesh based) with Large IBSI Phantom Image", 0.698, results["Volumetric Features::Surface to volume ratio (mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Compactness 1 (mesh, mesh based) with Large IBSI Phantom Image", 0.04105, results["Volumetric Features::Compactness 1 (mesh, mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Compactness 2 (mesh, mesh based) with Large IBSI Phantom Image", 0.599, results["Volumetric Features::Compactness 2 (mesh, mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Spherical disproportion (mesh, mesh based) with Large IBSI Phantom Image", 1.18, results["Volumetric Features::Spherical disproportion (mesh, mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Sphericity (mesh, mesh based) with Large IBSI Phantom Image", 0.843, results["Volumetric Features::Sphericity (mesh, mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Asphericity (mesh based) with Large IBSI Phantom Image", 0.186, results["Volumetric Features::Asphericity (mesh, mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Centre of mass shift with Large IBSI Phantom Image", 0.672, results["Volumetric Features::Centre of mass shift"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Maximum 3D diameter with Large IBSI Phantom Image", 11.66, results["Volumetric Features::Maximum 3D diameter"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::PCA Major axis length with Large IBSI Phantom Image", 11.40, results["Volumetric Features::PCA Major axis length"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::PCA Minor axis length with Large IBSI Phantom Image", 9.31, results["Volumetric Features::PCA Minor axis length"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::PCA Least axis length with Large IBSI Phantom Image", 8.54, results["Volumetric Features::PCA Least axis length"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::PCA Elongation with Large IBSI Phantom Image", 0.816, results["Volumetric Features::PCA Elongation"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::PCA Flatness with Large IBSI Phantom Image", 0.749, results["Volumetric Features::PCA Flatness"], 0.01);

    // These values are obtained by running the filter
    // They might be wrong!
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Voxel Volume with Large IBSI Phantom Image", 8, results["Volumetric Features::Voxel Volume"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Volume (voxel based) with Large IBSI Phantom Image", 592, results["Volumetric Features::Volume (voxel based)"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Surface (voxel based) with Large IBSI Phantom Image", 488, results["Volumetric Features::Surface (voxel based)"], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Centre of mass shift (uncorrected) with Large IBSI Phantom Image", 0.672, results["Volumetric Features::Centre of mass shift (uncorrected)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Bounding Box Volume with Large IBSI Phantom Image", 288, results["Volumetric Features::Bounding Box Volume"], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Surface to volume ratio (voxel based) with Large IBSI Phantom Image", 0.824, results["Volumetric Features::Surface to volume ratio (voxel based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Sphericity (voxel based) with Large IBSI Phantom Image", 0.699, results["Volumetric Features::Sphericity (voxel based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Asphericity (voxel based) with Large IBSI Phantom Image", 0.431, results["Volumetric Features::Asphericity (voxel based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Sphericity (mesh based) with Large IBSI Phantom Image", 0.879, results["Volumetric Features::Sphericity (mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Asphericity (mesh based) with Large IBSI Phantom Image", 0.138, results["Volumetric Features::Asphericity (mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Asphericity (mesh based) with Large IBSI Phantom Image", 0.138, results["Volumetric Features::Asphericity (mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Compactness 1 (voxel based) with Large IBSI Phantom Image", 0.031, results["Volumetric Features::Compactness 1 (voxel based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Compactness 1 old (voxel based) with Large IBSI Phantom Image", 5.388, results["Volumetric Features::Compactness 1 old (voxel based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Compactness 2 (voxel based) with Large IBSI Phantom Image", 0.341, results["Volumetric Features::Compactness 2 (voxel based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Compactness 1 (mesh based) with Large IBSI Phantom Image", 0.0437, results["Volumetric Features::Compactness 1 (mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Compactness 2 (mesh based) with Large IBSI Phantom Image", 0.678, results["Volumetric Features::Compactness 2 (mesh based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Spherical disproportion (voxel based) with Large IBSI Phantom Image", 1.43, results["Volumetric Features::Spherical disproportion (voxel based)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::PCA Major axis length (uncorrected) with Large IBSI Phantom Image", 11.40, results["Volumetric Features::PCA Major axis length (uncorrected)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::PCA Minor axis length (uncorrected) with Large IBSI Phantom Image", 9.31, results["Volumetric Features::PCA Minor axis length (uncorrected)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::PCA Least axis length (uncorrected) with Large IBSI Phantom Image", 8.54, results["Volumetric Features::PCA Least axis length (uncorrected)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::PCA Elongation (uncorrected) with Large IBSI Phantom Image", 0.816, results["Volumetric Features::PCA Elongation (uncorrected)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::PCA Flatness (uncorrected) with Large IBSI Phantom Image", 0.749, results["Volumetric Features::PCA Flatness (uncorrected)"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Volumetric Features::Compactness 1 old (mesh based) with Large IBSI Phantom Image", 6.278, results["Volumetric Features::Compactness 1 old (mesh based)"], 0.01);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFVolumetricStatistics )
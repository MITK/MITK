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

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkImageReadAccessor.h>

#include "mitkPAInSilicoTissueVolume.h"
#include "mitkPATissueGeneratorParameters.h"

class mitkPhotoacousticVolumeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticVolumeTestSuite);
  MITK_TEST(TestInitializedTissueContainsOnlyZeros);
  MITK_TEST(TestConvertedMitkImageContainsOnlyZerosOrAir);
  MITK_TEST(TestTissueVolumeContainsCorrectAbsorptionNumber);
  MITK_TEST(TestTissueVolumeContainsCorrectScatteringNumber);
  MITK_TEST(TestTissueVolumeContainsCorrectAnisotropyNumber);
  MITK_TEST(testSecondConstructor);
  MITK_TEST(testCompleteAirVoxelInclusion);
  MITK_TEST(testHalfAirVoxelInclusion);
  MITK_TEST(testCompleteAirAndSkinVoxelInclusion);
  MITK_TEST(testRandomizeCoefficients);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::pa::InSilicoTissueVolume::Pointer m_PhotoacousticVolume;
  mitk::pa::TissueGeneratorParameters::Pointer m_TissueGeneratorParameters;

public:

  void setUp() override
  {
    m_TissueGeneratorParameters = mitk::pa::TissueGeneratorParameters::New();
    m_PhotoacousticVolume = mitk::pa::InSilicoTissueVolume::New(m_TissueGeneratorParameters);
  }

  void TestInitializedTissueContainsOnlyZeros()
  {
    int dims = 30;
    m_TissueGeneratorParameters->SetXDim(dims);
    m_TissueGeneratorParameters->SetYDim(dims);
    m_TissueGeneratorParameters->SetZDim(dims);
    m_TissueGeneratorParameters->SetAirThicknessInMillimeters(0);
    m_TissueGeneratorParameters->SetBackgroundAbsorption(0);
    m_PhotoacousticVolume = mitk::pa::InSilicoTissueVolume::New(m_TissueGeneratorParameters);

    for (int x = 0; x < dims; x++)
    {
      for (int y = 0; y < dims; y++)
      {
        for (int z = 0; z < dims; z++)
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Every field should be initialized with 0.", std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(x, y, z)) < mitk::eps, true);
        }
      }
    }
  }

  void TestConvertedMitkImageContainsOnlyZerosOrAir()
  {
    int dims = 30;
    m_TissueGeneratorParameters->SetXDim(dims);
    m_TissueGeneratorParameters->SetYDim(dims);
    m_TissueGeneratorParameters->SetZDim(dims);
    m_PhotoacousticVolume = mitk::pa::InSilicoTissueVolume::New(m_TissueGeneratorParameters);

    mitk::Image::Pointer testImage = m_PhotoacousticVolume->ConvertToMitkImage();

    mitk::ImageReadAccessor imgMemAcc(testImage);
    auto* imagePointer = (double*)imgMemAcc.GetData();

    for (int index = 0; index < dims*dims*dims; index++, imagePointer++)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Every voxel in image should be 0.1 or 0.0001.", true, std::abs(*imagePointer - 0.1) <= mitk::eps || std::abs(*imagePointer - 0.0001) <= mitk::eps);
    }
  }

  void TestTissueVolumeContainsCorrectAbsorptionNumber()
  {
    int dims = 2;
    m_TissueGeneratorParameters->SetXDim(dims);
    m_TissueGeneratorParameters->SetYDim(dims);
    m_TissueGeneratorParameters->SetZDim(dims);
    m_PhotoacousticVolume = mitk::pa::InSilicoTissueVolume::New(m_TissueGeneratorParameters);

    m_PhotoacousticVolume->SetVolumeValues(0, 0, 0, 0, 0, 0);
    m_PhotoacousticVolume->SetVolumeValues(0, 0, 1, 1, 0, 0);
    m_PhotoacousticVolume->SetVolumeValues(0, 1, 0, 2, 0, 0);
    m_PhotoacousticVolume->SetVolumeValues(0, 1, 1, 3, 0, 0);
    m_PhotoacousticVolume->SetVolumeValues(1, 0, 0, 4, 0, 0);
    m_PhotoacousticVolume->SetVolumeValues(1, 0, 1, 5, 0, 0);
    m_PhotoacousticVolume->SetVolumeValues(1, 1, 0, 6, 0, 0);
    m_PhotoacousticVolume->SetVolumeValues(1, 1, 1, 7, 0, 0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 0.0, m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 1.0, m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(0, 0, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 2.0, m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 3.0, m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(0, 1, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 4.0, m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 5.0, m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 0, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 6.0, m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 7.0, m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 1, 1));
  }

  void TestTissueVolumeContainsCorrectScatteringNumber()
  {
    int dims = 2;
    m_TissueGeneratorParameters->SetXDim(dims);
    m_TissueGeneratorParameters->SetYDim(dims);
    m_TissueGeneratorParameters->SetZDim(dims);
    m_PhotoacousticVolume = mitk::pa::InSilicoTissueVolume::New(m_TissueGeneratorParameters);

    m_PhotoacousticVolume->SetVolumeValues(0, 0, 0, 0, 0, 0);
    m_PhotoacousticVolume->SetVolumeValues(0, 0, 1, 0, 1, 0);
    m_PhotoacousticVolume->SetVolumeValues(0, 1, 0, 0, 2, 0);
    m_PhotoacousticVolume->SetVolumeValues(0, 1, 1, 0, 3, 0);
    m_PhotoacousticVolume->SetVolumeValues(1, 0, 0, 0, 4, 0);
    m_PhotoacousticVolume->SetVolumeValues(1, 0, 1, 0, 5, 0);
    m_PhotoacousticVolume->SetVolumeValues(1, 1, 0, 0, 6, 0);
    m_PhotoacousticVolume->SetVolumeValues(1, 1, 1, 0, 7, 0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 0.0, m_PhotoacousticVolume->GetScatteringVolume()->GetData(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 1.0, m_PhotoacousticVolume->GetScatteringVolume()->GetData(0, 0, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 2.0, m_PhotoacousticVolume->GetScatteringVolume()->GetData(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 3.0, m_PhotoacousticVolume->GetScatteringVolume()->GetData(0, 1, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 4.0, m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 5.0, m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 0, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 6.0, m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 7.0, m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 1, 1));
  }

  void TestTissueVolumeContainsCorrectAnisotropyNumber()
  {
    int dims = 2;
    m_TissueGeneratorParameters->SetXDim(dims);
    m_TissueGeneratorParameters->SetYDim(dims);
    m_TissueGeneratorParameters->SetZDim(dims);
    m_PhotoacousticVolume = mitk::pa::InSilicoTissueVolume::New(m_TissueGeneratorParameters);

    m_PhotoacousticVolume->SetVolumeValues(0, 0, 0, 0, 0, 0);
    m_PhotoacousticVolume->SetVolumeValues(0, 0, 1, 0, 0, 1);
    m_PhotoacousticVolume->SetVolumeValues(0, 1, 0, 0, 0, 2);
    m_PhotoacousticVolume->SetVolumeValues(0, 1, 1, 0, 0, 3);
    m_PhotoacousticVolume->SetVolumeValues(1, 0, 0, 0, 0, 4);
    m_PhotoacousticVolume->SetVolumeValues(1, 0, 1, 0, 0, 5);
    m_PhotoacousticVolume->SetVolumeValues(1, 1, 0, 0, 0, 6);
    m_PhotoacousticVolume->SetVolumeValues(1, 1, 1, 0, 0, 7);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 0.0, m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 1.0, m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(0, 0, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 2.0, m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 3.0, m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(0, 1, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 4.0, m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 5.0, m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 0, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 6.0, m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be correct value", 7.0, m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 1, 1));
  }

  mitk::pa::Volume::Pointer createTestVolume(double value)
  {
    auto* data = new double[27];
    for (int i = 0; i < 27; ++i)
      data[i] = value;
    return mitk::pa::Volume::New(data, 3, 3, 3);
  }

  void assertEqual(mitk::pa::Volume::Pointer first, mitk::pa::Volume::Pointer second)
  {
    CPPUNIT_ASSERT(first->GetXDim() == second->GetXDim());
    CPPUNIT_ASSERT(first->GetYDim() == second->GetYDim());
    CPPUNIT_ASSERT(first->GetZDim() == second->GetZDim());
    for (unsigned int x = 0; x < first->GetXDim(); ++x)
      for (unsigned int y = 0; y < first->GetYDim(); ++y)
        for (unsigned int z = 0; z < first->GetZDim(); ++z)
          CPPUNIT_ASSERT(std::abs(first->GetData(x, y, z) - second->GetData(x, y, z)) < mitk::eps);
  }

  void testSecondConstructor()
  {
    mitk::pa::Volume::Pointer absorption = createTestVolume(1);
    mitk::pa::Volume::Pointer scattering = createTestVolume(2);
    mitk::pa::Volume::Pointer anisotropy = createTestVolume(3);
    mitk::pa::Volume::Pointer segmentation = createTestVolume(4);
    mitk::PropertyList::Pointer properties = mitk::PropertyList::New();

    m_PhotoacousticVolume = mitk::pa::InSilicoTissueVolume::New(absorption,
      scattering, anisotropy, segmentation, m_TissueGeneratorParameters, properties);

    assertEqual(m_PhotoacousticVolume->GetAbsorptionVolume(), absorption);
    assertEqual(m_PhotoacousticVolume->GetScatteringVolume(), scattering);
    assertEqual(m_PhotoacousticVolume->GetAnisotropyVolume(), anisotropy);
    assertEqual(m_PhotoacousticVolume->GetSegmentationVolume(), segmentation);
  }

  void testCompleteAirVoxelInclusion()
  {
    mitk::pa::Volume::Pointer absorption = createTestVolume(1);
    mitk::pa::Volume::Pointer scattering = createTestVolume(2);
    mitk::pa::Volume::Pointer anisotropy = createTestVolume(3);
    mitk::pa::Volume::Pointer segmentation = createTestVolume(4);
    mitk::PropertyList::Pointer properties = mitk::PropertyList::New();
    m_TissueGeneratorParameters->SetXDim(3);
    m_TissueGeneratorParameters->SetYDim(3);
    m_TissueGeneratorParameters->SetZDim(3);
    m_TissueGeneratorParameters->SetAirThicknessInMillimeters(10);
    m_TissueGeneratorParameters->SetSkinThicknessInMillimeters(0);
    m_TissueGeneratorParameters->SetAirAbsorption(2);
    m_TissueGeneratorParameters->SetAirScattering(4);
    m_TissueGeneratorParameters->SetAirAnisotropy(6);

    m_PhotoacousticVolume = mitk::pa::InSilicoTissueVolume::New(absorption,
      scattering, anisotropy, segmentation, m_TissueGeneratorParameters, properties);

    m_PhotoacousticVolume->FinalizeVolume();

    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(0, 0, 0) - 2) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 1, 1) - 1) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 1, 2) - 1) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(0, 0, 0) - 4) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 1, 1) - 2) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 1, 2) - 2) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(0, 0, 0) - 6) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 1, 1) - 3) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 1, 2) - 3) < mitk::eps);
  }

  void testRandomizeCoefficients()
  {
    mitk::pa::Volume::Pointer absorption = createTestVolume(1);
    mitk::pa::Volume::Pointer scattering = createTestVolume(1);
    mitk::pa::Volume::Pointer anisotropy = createTestVolume(1);
    mitk::pa::Volume::Pointer segmentation = createTestVolume(4);
    mitk::PropertyList::Pointer properties = mitk::PropertyList::New();
    m_TissueGeneratorParameters->SetXDim(3);
    m_TissueGeneratorParameters->SetYDim(3);
    m_TissueGeneratorParameters->SetZDim(3);
    m_TissueGeneratorParameters->SetRandomizePhysicalProperties(true);
    m_TissueGeneratorParameters->SetRandomizePhysicalPropertiesPercentage(1);
    m_TissueGeneratorParameters->SetRngSeed(17);
    m_TissueGeneratorParameters->SetUseRngSeed(true);

    m_PhotoacousticVolume = mitk::pa::InSilicoTissueVolume::New(absorption,
      scattering, anisotropy, segmentation, m_TissueGeneratorParameters, properties);

    m_PhotoacousticVolume->FinalizeVolume();

    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(0, 0, 0) - 1) < 0.1);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 1, 1) - 1) < 0.1);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 1, 2) - 1) < 0.1);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(0, 0, 0) - 1) < 0.1);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 1, 1) - 1) < 0.1);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 1, 2) - 1) < 0.1);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(0, 0, 0) - 1) < 0.1);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 1, 1) - 1) < 0.1);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 1, 2) - 1) < 0.1);
  }

  void testCompleteAirAndSkinVoxelInclusion()
  {
    mitk::pa::Volume::Pointer absorption = createTestVolume(1);
    mitk::pa::Volume::Pointer scattering = createTestVolume(2);
    mitk::pa::Volume::Pointer anisotropy = createTestVolume(3);
    mitk::pa::Volume::Pointer segmentation = createTestVolume(4);
    mitk::PropertyList::Pointer properties = mitk::PropertyList::New();
    m_TissueGeneratorParameters->SetXDim(3);
    m_TissueGeneratorParameters->SetYDim(3);
    m_TissueGeneratorParameters->SetZDim(3);
    m_TissueGeneratorParameters->SetAirThicknessInMillimeters(10);
    m_TissueGeneratorParameters->SetSkinThicknessInMillimeters(10);
    m_TissueGeneratorParameters->SetAirAbsorption(2);
    m_TissueGeneratorParameters->SetAirScattering(4);
    m_TissueGeneratorParameters->SetAirAnisotropy(6);
    m_TissueGeneratorParameters->SetSkinAbsorption(4);
    m_TissueGeneratorParameters->SetSkinScattering(8);
    m_TissueGeneratorParameters->SetSkinAnisotropy(12);

    m_PhotoacousticVolume = mitk::pa::InSilicoTissueVolume::New(absorption,
      scattering, anisotropy, segmentation, m_TissueGeneratorParameters, properties);

    m_PhotoacousticVolume->FinalizeVolume();

    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(0, 0, 0) - 2) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 1, 1) - 4) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 1, 2) - 1) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(0, 0, 0) - 4) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 1, 1) - 8) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 1, 2) - 2) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(0, 0, 0) - 6) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 1, 1) - 12) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 1, 2) - 3) < mitk::eps);
  }

  void testHalfAirVoxelInclusion()
  {
    mitk::pa::Volume::Pointer absorption = createTestVolume(1);
    mitk::pa::Volume::Pointer scattering = createTestVolume(2);
    mitk::pa::Volume::Pointer anisotropy = createTestVolume(3);
    mitk::pa::Volume::Pointer segmentation = createTestVolume(4);
    mitk::PropertyList::Pointer properties = mitk::PropertyList::New();
    m_TissueGeneratorParameters->SetXDim(3);
    m_TissueGeneratorParameters->SetYDim(3);
    m_TissueGeneratorParameters->SetZDim(3);
    m_TissueGeneratorParameters->SetAirThicknessInMillimeters(15);
    m_TissueGeneratorParameters->SetSkinThicknessInMillimeters(0);
    m_TissueGeneratorParameters->SetAirAbsorption(2);
    m_TissueGeneratorParameters->SetAirScattering(4);
    m_TissueGeneratorParameters->SetAirAnisotropy(6);

    m_PhotoacousticVolume = mitk::pa::InSilicoTissueVolume::New(absorption,
      scattering, anisotropy, segmentation, m_TissueGeneratorParameters, properties);

    m_PhotoacousticVolume->FinalizeVolume();

    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(0, 0, 0) - 2) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 1, 1) - 1.5) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAbsorptionVolume()->GetData(1, 1, 2) - 1) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(0, 0, 0) - 4) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 1, 1) - 3) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetScatteringVolume()->GetData(1, 1, 2) - 2) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(0, 0, 0) - 6) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 1, 1) - 4.5) < mitk::eps);
    CPPUNIT_ASSERT(std::abs(m_PhotoacousticVolume->GetAnisotropyVolume()->GetData(1, 1, 2) - 3) < mitk::eps);
  }

  void tearDown() override
  {
    m_PhotoacousticVolume = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticVolume)

///*===================================================================

//The Medical Imaging Interaction Toolkit (MITK)

//Copyright (c) German Cancer Research Center,
//Division of Medical and Biological Informatics.
//All rights reserved.

//This software is distributed WITHOUT ANY WARRANTY; without
//even the implied warranty of MERCHANTABILITY or FITNESS FOR
//A PARTICULAR PURPOSE.

//See LICENSE.txt or http://www.mitk.org for details.

//===================================================================*/
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkPASlicedVolumeGenerator.h>

class mitkSlicedVolumeGeneratorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSlicedVolumeGeneratorTestSuite);
  MITK_TEST(testConstructorDestructor);
  MITK_TEST(testGetSlicedFluenceVolume);
  MITK_TEST(testGetSlicedFluenceVolumeInverse);
  MITK_TEST(testGetSlicedFluenceVolumeWithPrecorrection);
  MITK_TEST(testGetSlicedFluenceVolumeWithPrecorrectionInverse);
  MITK_TEST(testGetSlicedSignalVolume);
  MITK_TEST(testGetSlicedAbsorptionVolume);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::pa::ComposedVolume::Pointer m_ComposedVolume;
  mitk::pa::TissueGeneratorParameters::Pointer m_DefaultParameters;
  mitk::pa::InSilicoTissueVolume::Pointer m_InSilicoTissueVolume;
  mitk::pa::SlicedVolumeGenerator::Pointer m_SlicedVolumeGenerator;
  mitk::pa::Volume::Pointer m_PrecorrectionVolume;

public:

  void setUp() override
  {
    m_SlicedVolumeGenerator = nullptr;
    m_DefaultParameters = mitk::pa::TissueGeneratorParameters::New();
    m_DefaultParameters->SetXDim(3);
    m_DefaultParameters->SetYDim(3);
    m_DefaultParameters->SetZDim(3);
    m_InSilicoTissueVolume = mitk::pa::InSilicoTissueVolume::New(m_DefaultParameters);
    m_ComposedVolume = mitk::pa::ComposedVolume::New(m_InSilicoTissueVolume);
    m_ComposedVolume->AddSlice(CreateValidationPair(-1, 1));
    m_ComposedVolume->AddSlice(CreateValidationPair(0, 3));
    m_ComposedVolume->AddSlice(CreateValidationPair(1, 6));
    m_PrecorrectionVolume = CreatePrecorrectionVolume();
  }

  mitk::pa::Volume::Pointer CreatePrecorrectionVolume()
  {
    auto* data = new double[27];
    for (int i = 0; i < 27; ++i)
      data[i] = 0.5;
    return mitk::pa::Volume::New(data, 3, 3, 3);
  }

  void FillYSliceWith(mitk::pa::Volume::Pointer fluenceVolume, double ySlice, double value)
  {
    for (unsigned int x = 0; x < fluenceVolume->GetXDim(); ++x)
      for (unsigned int z = 0; z < fluenceVolume->GetZDim(); ++z)
      {
        fluenceVolume->SetData(value, x, ySlice, z);
      }
  }

  mitk::pa::FluenceYOffsetPair::Pointer CreateValidationPair(double yOffset, int start)
  {
    auto* data = new double[27];
    mitk::pa::Volume::Pointer fluenceVolume = mitk::pa::Volume::New(data, 3, 3, 3);

    FillYSliceWith(fluenceVolume, 0, start + 0);
    FillYSliceWith(fluenceVolume, 1, start + 1);
    FillYSliceWith(fluenceVolume, 2, start + 2);

    return mitk::pa::FluenceYOffsetPair::New(fluenceVolume, yOffset);
  }

  void AssertYSliceValue(mitk::pa::Volume::Pointer fluenceVolume, double ySlice, double value)
  {
    for (unsigned int x = 0; x < fluenceVolume->GetXDim(); ++x)
      for (unsigned int z = 0; z < fluenceVolume->GetZDim(); ++z)
      {
        std::string msg = "Expected: " + std::to_string(value) + " actual: " + std::to_string(fluenceVolume->GetData(x, ySlice, z));
        CPPUNIT_ASSERT_MESSAGE(msg, std::abs(fluenceVolume->GetData(x, ySlice, z) - value) < mitk::eps);
      }
  }

  void testConstructorDestructor()
  {
    m_SlicedVolumeGenerator = mitk::pa::SlicedVolumeGenerator::New(0, false, nullptr, false);
    CPPUNIT_ASSERT(m_SlicedVolumeGenerator.IsNotNull());
  }

  void testGetSlicedFluenceVolume()
  {
    m_SlicedVolumeGenerator = mitk::pa::SlicedVolumeGenerator::New(1, false, nullptr, false);
    mitk::pa::Volume::Pointer slicedFluence =
      m_SlicedVolumeGenerator->GetSlicedFluenceImageFromComposedVolume(m_ComposedVolume);
    CPPUNIT_ASSERT(slicedFluence->GetXDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetYDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetZDim() == 3);
    AssertYSliceValue(slicedFluence, 0, 1);
    AssertYSliceValue(slicedFluence, 1, 4);
    AssertYSliceValue(slicedFluence, 2, 8);
  }

  void testGetSlicedFluenceVolumeInverse()
  {
    m_SlicedVolumeGenerator = mitk::pa::SlicedVolumeGenerator::New(1, false, nullptr, true);
    mitk::pa::Volume::Pointer slicedFluence =
      m_SlicedVolumeGenerator->GetSlicedFluenceImageFromComposedVolume(m_ComposedVolume);
    CPPUNIT_ASSERT(slicedFluence->GetXDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetYDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetZDim() == 3);
    AssertYSliceValue(slicedFluence, 0, 1);
    AssertYSliceValue(slicedFluence, 1, 1.0 / 4.0);
    AssertYSliceValue(slicedFluence, 2, 1.0 / 8.0);
  }

  void testGetSlicedFluenceVolumeWithPrecorrection()
  {
    m_SlicedVolumeGenerator = mitk::pa::SlicedVolumeGenerator::New(1, true, m_PrecorrectionVolume, false);
    mitk::pa::Volume::Pointer slicedFluence =
      m_SlicedVolumeGenerator->GetSlicedFluenceImageFromComposedVolume(m_ComposedVolume);
    CPPUNIT_ASSERT(slicedFluence->GetXDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetYDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetZDim() == 3);
    AssertYSliceValue(slicedFluence, 0, 2);
    AssertYSliceValue(slicedFluence, 1, 8);
    AssertYSliceValue(slicedFluence, 2, 16);
  }

  void testGetSlicedFluenceVolumeWithPrecorrectionInverse()
  {
    m_SlicedVolumeGenerator = mitk::pa::SlicedVolumeGenerator::New(1, true, m_PrecorrectionVolume, true);
    mitk::pa::Volume::Pointer slicedFluence =
      m_SlicedVolumeGenerator->GetSlicedFluenceImageFromComposedVolume(m_ComposedVolume);
    CPPUNIT_ASSERT(slicedFluence->GetXDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetYDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetZDim() == 3);
    AssertYSliceValue(slicedFluence, 0, 1.0 / 2);
    AssertYSliceValue(slicedFluence, 1, 1.0 / 8);
    AssertYSliceValue(slicedFluence, 2, 1.0 / 16);
  }

  void testGetSlicedSignalVolume()
  {
    m_SlicedVolumeGenerator = mitk::pa::SlicedVolumeGenerator::New(1, false, nullptr, false);
    mitk::pa::Volume::Pointer slicedFluence =
      m_SlicedVolumeGenerator->GetSlicedSignalImageFromComposedVolume(m_ComposedVolume);
    CPPUNIT_ASSERT(slicedFluence->GetXDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetYDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetZDim() == 3);
    AssertYSliceValue(slicedFluence, 0, 1 * m_DefaultParameters->GetBackgroundAbsorption());
    AssertYSliceValue(slicedFluence, 1, 4 * m_DefaultParameters->GetBackgroundAbsorption());
    AssertYSliceValue(slicedFluence, 2, 8 * m_DefaultParameters->GetBackgroundAbsorption());
  }

  void testGetSlicedAbsorptionVolume()
  {
    m_SlicedVolumeGenerator = mitk::pa::SlicedVolumeGenerator::New(1, false, nullptr, false);
    mitk::pa::Volume::Pointer slicedFluence =
      m_SlicedVolumeGenerator->GetSlicedGroundTruthImageFromComposedVolume(m_ComposedVolume);
    CPPUNIT_ASSERT(slicedFluence->GetXDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetYDim() == 3);
    CPPUNIT_ASSERT(slicedFluence->GetZDim() == 3);
    AssertYSliceValue(slicedFluence, 0, m_DefaultParameters->GetBackgroundAbsorption());
    AssertYSliceValue(slicedFluence, 1, m_DefaultParameters->GetBackgroundAbsorption());
    AssertYSliceValue(slicedFluence, 2, m_DefaultParameters->GetBackgroundAbsorption());
  }

  void tearDown() override
  {
    m_SlicedVolumeGenerator = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSlicedVolumeGenerator)

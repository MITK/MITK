/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkPhotoacousticFilterService.h>
#include <mitkImage.h>
#include <mitkImageReadAccessor.h>
#include <mitkBeamformingSettings.h>

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResourceStream.h>

#include <random>
#include <chrono>

class mitkPAFilterServiceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPAFilterServiceTestSuite);
  MITK_TEST(testRunning);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::PhotoacousticFilterService::Pointer m_PhotoacousticFilterService;
  mitk::BeamformingSettings::Pointer m_BeamformingSettings;
  unsigned int* inputDimensions;
  const int xDim = 16;
  const int yDim = 128;
  const int length = yDim * xDim;

public:

  void setUp() override
  {
    m_PhotoacousticFilterService = mitk::PhotoacousticFilterService::New();
    m_BeamformingSettings = CreateBeamformingSettings();
  }

  mitk::BeamformingSettings::Pointer CreateBeamformingSettings()
  {
    inputDimensions = new unsigned int[length];
    inputDimensions[0] = yDim;
    inputDimensions[1] = xDim;
    mitk::BeamformingSettings::Pointer outputSettings = mitk::BeamformingSettings::New(
      (float)(0.3 / 1000),
      (float)(1500),
      (float)(0.0125 / 1000000),
      27,
      true,
      3000,
      128,
      inputDimensions,
      yDim * (0.0125 / 1000000) * (1500),
      false,
      16,
      mitk::BeamformingSettings::Apodization::Box,
      128,
      mitk::BeamformingSettings::BeamformingAlgorithm::DAS,
      mitk::BeamformingSettings::ProbeGeometry::Linear,
      0);

    return outputSettings;
  }

  void testRunning()
  {
    float* testArray = new float[length];
    for (int i = 0; i < length; ++i)
    {
      testArray[i] = 0;
    }

    mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();
    mitk::Image::Pointer testImage = mitk::Image::New();
    testImage->Initialize(pixelType, 2, inputDimensions);
    testImage->SetImportSlice(testArray, 0, 0, 0, mitk::Image::ImportMemoryManagementType::CopyMemory);
    delete[] testArray;

    mitk::ImageReadAccessor readAccessInput(testImage);
    const float* inputArray = (const float*)readAccessInput.GetData();

    for (int i = 0; i < length; ++i)
    {
      CPPUNIT_ASSERT_MESSAGE(std::string("Input array already not correct: " + std::to_string(inputArray[i])), std::abs(inputArray[i]) < 1e-5f);
    }

    auto output = m_PhotoacousticFilterService->ApplyBeamforming(testImage, m_BeamformingSettings);

    mitk::ImageReadAccessor readAccess(output);
    const float* outputArray = (const float*)readAccess.GetData();

    for (int i = 0; i < length; ++i)
    {
      CPPUNIT_ASSERT_MESSAGE(std::string("Output array not correct: " + std::to_string(std::abs(outputArray[i]))), std::abs(outputArray[i]) < 1e-5f);
    }
  }

  void tearDown() override
  {
    m_PhotoacousticFilterService = nullptr;
    delete[] inputDimensions;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPAFilterService)

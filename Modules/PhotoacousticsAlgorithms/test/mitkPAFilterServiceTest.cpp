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

public:

  void setUp() override
  {
    m_PhotoacousticFilterService = mitk::PhotoacousticFilterService::New();
  }

  mitk::BeamformingSettings::Pointer CreateEmptyBeamformingSettings()
  {
    mitk::BeamformingSettings::Pointer config = mitk::BeamformingSettings::New();
    return config;
  }

  mitk::BeamformingSettings::Pointer CreateFunctionalBeamformingSettings()
  {
    mitk::BeamformingSettings::Pointer config = mitk::BeamformingSettings::New();
    return config;
  }

  void testRunning()
  {
    int xDim = 3;
    int yDim = 3;
    int length = yDim * xDim;
    float* testArray = new float[length];
    for (int i = 0; i < length; ++i)
    {
      testArray[i] = 0;
    }

    unsigned int NUMBER_OF_SPATIAL_DIMENSIONS = 2;
    unsigned int* dimensions = new unsigned int[NUMBER_OF_SPATIAL_DIMENSIONS];
    dimensions[0] = yDim;
    dimensions[1] = xDim;
    mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();
    mitk::Image::Pointer testImage = mitk::Image::New();
    testImage->Initialize(pixelType, NUMBER_OF_SPATIAL_DIMENSIONS, dimensions);
    testImage->SetImportSlice(testArray, 0, 0, 0, mitk::Image::ImportMemoryManagementType::CopyMemory);
    delete[] testArray;

    mitk::ImageReadAccessor readAccessInput(testImage);
    const float* inputArray = (const float*)readAccessInput.GetData();

    for (int i = 0; i < length; ++i)
    {
      CPPUNIT_ASSERT_MESSAGE(std::string("Input array already not correct: " + std::to_string(inputArray[i])), abs(inputArray[i]) < 1e-5f);
    }

    mitk::BeamformingSettings::Pointer config = CreateEmptyBeamformingSettings();
    auto output = m_PhotoacousticFilterService->ApplyBeamforming(testImage, config);

    mitk::ImageReadAccessor readAccess(output);
    const float* outputArray = (const float*)readAccess.GetData();

    for (int i = 0; i < length; ++i)
    {
      CPPUNIT_ASSERT_MESSAGE(std::string("Ouput array not correct: " + std::to_string(abs(outputArray[i]))), abs(outputArray[i]) < 1e-5f);
    }
    delete[] dimensions;
  }

  void tearDown() override
  {
    m_PhotoacousticFilterService = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPAFilterService)

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

#include "mitkPATissueGenerator.h"

class mitkPhotoacousticTissueGeneratorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticTissueGeneratorTestSuite);
  MITK_TEST(testCallWithEmptyParameters);
  MITK_TEST(testCallWithWorkingParameters);
  CPPUNIT_TEST_SUITE_END();

private:

public:

  void setUp() override
  {
  }

  mitk::pa::TissueGeneratorParameters::Pointer createRandomTestVolumeParameters()
  {
    auto returnParameters = mitk::pa::TissueGeneratorParameters::New();

    returnParameters->SetXDim(rand() % 50 + 1);
    returnParameters->SetYDim(rand() % 50 + 1);
    returnParameters->SetZDim(rand() % 50 + 1);
    returnParameters->SetBackgroundAbsorption(rand() % 100 / 10.0);
    returnParameters->SetBackgroundScattering(rand() % 100 / 10.0);
    returnParameters->SetBackgroundAnisotropy(rand() % 100 / 10.0);
    int min = rand() % 10;
    returnParameters->SetMinNumberOfVessels(min);
    returnParameters->SetMaxNumberOfVessels(min + (rand() % 10));
    returnParameters->SetCalculateNewVesselPositionCallback(
      &mitk::pa::VesselMeanderStrategy::CalculateRandomlyDivergingPosition);
    returnParameters->SetMinVesselZOrigin(rand() % 3 + 1);
    returnParameters->SetMaxVesselZOrigin(rand() % 3 + 1);
    int minRad = rand() % 100;
    returnParameters->SetMinVesselRadiusInMillimeters(minRad);
    returnParameters->SetMaxVesselRadiusInMillimeters(minRad + (rand() % 100));
    returnParameters->SetVoxelSpacingInCentimeters(1);
    return returnParameters;
  }

  void testCallWithEmptyParameters()
  {
    auto parameters = mitk::pa::TissueGeneratorParameters::New();
    auto volume = mitk::pa::InSilicoTissueGenerator::GenerateInSilicoData(parameters);
    CPPUNIT_ASSERT(volume.IsNotNull());
  }

  void testCallWithWorkingParameters()
  {
    for (int i = 0; i < 20; i++)
    {
      auto parameters = createRandomTestVolumeParameters();
      auto volume = mitk::pa::InSilicoTissueGenerator::GenerateInSilicoData(parameters);
      CPPUNIT_ASSERT(volume.IsNotNull());
    }
  }

  void tearDown() override
  {
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticTissueGenerator)

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
#include "mitkPAVector.h"
#include "mitkPAVessel.h"
#include "mitkIOUtil.h"

class mitkPhotoacousticVesselTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticVesselTestSuite);
  MITK_TEST(testEmptyInitializationProperties);
  MITK_TEST(testWalkInStraightLine);
  MITK_TEST(testBifurcate);
  MITK_TEST(testPartialVolume);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::pa::Vessel::Pointer m_TestVessel;
  mitk::pa::Vessel::CalculateNewVesselPositionCallback m_StraightLine;
  mitk::pa::Vessel::CalculateNewVesselPositionCallback m_Diverging;
  mitk::pa::InSilicoTissueVolume::Pointer m_TestInSilicoVolume;
  mitk::pa::TissueGeneratorParameters::Pointer m_TestVolumeParameters;

public:

  void setUp()
  {
    auto params = mitk::pa::VesselProperties::New();
    m_TestVessel = mitk::pa::Vessel::New(params);
    m_StraightLine = &mitk::pa::VesselMeanderStrategy::CalculateNewPositionInStraightLine;
    m_Diverging = &mitk::pa::VesselMeanderStrategy::CalculateRandomlyDivergingPosition;
    m_TestVolumeParameters = createTestVolumeParameters();
    m_TestInSilicoVolume = mitk::pa::InSilicoTissueVolume::New(m_TestVolumeParameters);
  }

  mitk::pa::TissueGeneratorParameters::Pointer createTestVolumeParameters()
  {
    auto returnParameters =
      mitk::pa::TissueGeneratorParameters::New();

    returnParameters->SetXDim(10);
    returnParameters->SetYDim(10);
    returnParameters->SetZDim(10);
    returnParameters->SetBackgroundAbsorption(0);
    returnParameters->SetBackgroundScattering(0);
    returnParameters->SetBackgroundAnisotropy(0);
    return returnParameters;
  }

  void testEmptyInitializationProperties()
  {
    CPPUNIT_ASSERT(m_TestVessel->CanBifurcate() == false);
    CPPUNIT_ASSERT(m_TestVessel->IsFinished() == true);
  }

  void testPartialVolume()
  {
    auto testPosition = mitk::pa::Vector::New();
    testPosition->SetElement(0, 0);
    testPosition->SetElement(1, 4);
    testPosition->SetElement(2, 4);
    auto testDirection = mitk::pa::Vector::New();
    testDirection->SetElement(0, 1);
    testDirection->SetElement(1, 0);
    testDirection->SetElement(2, 0);
    auto params = mitk::pa::VesselProperties::New();
    params->SetDoPartialVolume(true);
    params->SetRadiusInVoxel(1);
    params->SetBifurcationFrequency(100);
    params->SetAbsorptionCoefficient(10);
    params->SetScatteringCoefficient(10);
    params->SetAnisotopyCoefficient(10);
    params->SetPositionVector(testPosition);
    params->SetDirectionVector(testDirection);
    m_TestVessel = mitk::pa::Vessel::New(params);

    CPPUNIT_ASSERT(m_TestVessel->CanBifurcate() == false);
    CPPUNIT_ASSERT(m_TestVessel->IsFinished() == false);

    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 4)) <= mitk::eps);

    m_TestVessel->ExpandVessel(m_TestInSilicoVolume, m_StraightLine, 0, nullptr);

    CPPUNIT_ASSERT_MESSAGE(std::to_string(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 4)),
      abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 4) - 10) <= mitk::eps);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 5)),
      abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 5) - 10) <= mitk::eps);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 3)),
      abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 3) - 10) <= mitk::eps);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 5, 4)),
      abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 5, 4) - 10) <= mitk::eps);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 3, 4)),
      abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 3, 4) - 10) <= mitk::eps);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 5, 5)),
      abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 5, 5) - 5.85786438) <= 1e-6);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 3, 3)),
      abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 3, 3) - 5.85786438) <= 1e-6);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 5, 3)),
      abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 5, 3) - 5.85786438) <= 1e-6);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 3, 5)),
      abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 3, 5) - 5.85786438) <= 1e-6);
  }

  void testWalkInStraightLine()
  {
    auto testPosition = mitk::pa::Vector::New();
    testPosition->SetElement(0, 0);
    testPosition->SetElement(1, 4);
    testPosition->SetElement(2, 4);
    auto testDirection = mitk::pa::Vector::New();
    testDirection->SetElement(0, 1);
    testDirection->SetElement(1, 0);
    testDirection->SetElement(2, 0);
    auto params = mitk::pa::VesselProperties::New();
    params->SetDoPartialVolume(false);
    params->SetRadiusInVoxel(1);
    params->SetBifurcationFrequency(100);
    params->SetAbsorptionCoefficient(10);
    params->SetScatteringCoefficient(10);
    params->SetAnisotopyCoefficient(10);
    params->SetPositionVector(testPosition);
    params->SetDirectionVector(testDirection);
    m_TestVessel = mitk::pa::Vessel::New(params);

    CPPUNIT_ASSERT(m_TestVessel->CanBifurcate() == false);
    CPPUNIT_ASSERT(m_TestVessel->IsFinished() == false);

    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 4)) <= mitk::eps);

    m_TestVessel->ExpandVessel(m_TestInSilicoVolume, m_StraightLine, 0, nullptr);

    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 4) - 10) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 5, 4)) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 6, 4)) <= mitk::eps);

    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 4) - 10) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 5)) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 6)) <= mitk::eps);

    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 4) - 10) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(1, 4, 4) - 10) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(2, 4, 4)) <= mitk::eps);

    m_TestVessel->ExpandVessel(m_TestInSilicoVolume, m_StraightLine, 0, nullptr);

    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(1, 4, 4) - 10) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(1, 5, 4)) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(1, 6, 4)) <= mitk::eps);

    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(1, 4, 4) - 10) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(1, 4, 5)) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(1, 4, 6)) <= mitk::eps);

    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(1, 4, 4) - 10) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(2, 4, 4) - 10) <= mitk::eps);
    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(3, 4, 4)) <= mitk::eps);
  }

  void testBifurcate()
  {
    auto testPosition = mitk::pa::Vector::New();
    testPosition->SetElement(0, 0);
    testPosition->SetElement(1, 4);
    testPosition->SetElement(2, 4);
    auto testDirection = mitk::pa::Vector::New();
    testDirection->SetElement(0, 1);
    testDirection->SetElement(1, 0);
    testDirection->SetElement(2, 0);
    auto params = mitk::pa::VesselProperties::New();
    params->SetRadiusInVoxel(1);
    params->SetBifurcationFrequency(1);
    params->SetAbsorptionCoefficient(10);
    params->SetScatteringCoefficient(10);
    params->SetAnisotopyCoefficient(10);
    params->SetPositionVector(testPosition);
    params->SetDirectionVector(testDirection);
    m_TestVessel = mitk::pa::Vessel::New(params);

    CPPUNIT_ASSERT(m_TestVessel->CanBifurcate() == false);
    CPPUNIT_ASSERT(m_TestVessel->IsFinished() == false);

    CPPUNIT_ASSERT(abs(m_TestInSilicoVolume->GetAbsorptionVolume()->GetData(0, 4, 4)) <= mitk::eps);

    m_TestVessel->ExpandVessel(m_TestInSilicoVolume, m_StraightLine, 0, nullptr);
    m_TestVessel->ExpandVessel(m_TestInSilicoVolume, m_StraightLine, 0, nullptr);

    CPPUNIT_ASSERT(m_TestVessel->CanBifurcate() == true);
    std::mt19937 rng;
    auto bifurcationVessel = m_TestVessel->Bifurcate(&rng);

    CPPUNIT_ASSERT(m_TestVessel->CanBifurcate() == false);
    CPPUNIT_ASSERT(bifurcationVessel->CanBifurcate() == false);
  }

  void tearDown()
  {
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticVessel)

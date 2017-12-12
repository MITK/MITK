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

#include "mitkPAVesselTree.h"
#include "mitkPAInSilicoTissueVolume.h"

using namespace mitk::pa;

class mitkPhotoacousticVesselTreeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticVesselTreeTestSuite);
  MITK_TEST(testVesselTreeInitialBehavior);
  MITK_TEST(testCallStepMethod);
  CPPUNIT_TEST_SUITE_END();

private:

public:

  VesselTree::Pointer m_Tree;
  VesselProperties::Pointer m_VesselProperties;
  Vessel::CalculateNewVesselPositionCallback m_StraightLine;
  InSilicoTissueVolume::Pointer m_TestInSilicoVolume;

  void setUp() override
  {
    m_VesselProperties = VesselProperties::New();
    m_Tree = VesselTree::New(m_VesselProperties);
    m_StraightLine = &VesselMeanderStrategy::CalculateNewPositionInStraightLine;
    m_TestInSilicoVolume = InSilicoTissueVolume::New(createTestVolumeParameters());
  }

  TissueGeneratorParameters::Pointer createTestVolumeParameters()
  {
    auto returnParameters =
      TissueGeneratorParameters::New();

    returnParameters->SetXDim(10);
    returnParameters->SetYDim(10);
    returnParameters->SetZDim(10);
    returnParameters->SetBackgroundAbsorption(0);
    returnParameters->SetBackgroundScattering(0);
    returnParameters->SetBackgroundAnisotropy(0);
    return returnParameters;
  }

  void testVesselTreeInitialBehavior()
  {
    CPPUNIT_ASSERT(m_Tree->IsFinished() == true);
  }

  void testCallStepMethod()
  {
    //really bad test atm.. The only thing that is tested is that the method can be called without a crash.
    //But hey - it is something :P
    m_VesselProperties->SetRadiusInVoxel(2);
    std::mt19937 rng;
    rng.seed(1);
    m_Tree = VesselTree::New(m_VesselProperties);
    m_Tree->Step(m_TestInSilicoVolume, m_StraightLine, 0, &rng);

    rng.seed(1);
    auto secondTree = VesselTree::New(m_VesselProperties);
    secondTree->Step(m_TestInSilicoVolume, m_StraightLine, 0, &rng);

    CPPUNIT_ASSERT(Equal(m_Tree, secondTree, 1e-6, true));

    secondTree->Step(m_TestInSilicoVolume, m_StraightLine, 0, &rng);

    CPPUNIT_ASSERT(!Equal(m_Tree, secondTree, 1e-6, true));

    int i = 0;
    for (; i < 1000; i++)
    {
      secondTree->Step(m_TestInSilicoVolume, m_StraightLine, 0, &rng);
      if (secondTree->IsFinished())
        break;
    }

    CPPUNIT_ASSERT(i < 999);
  }

  void tearDown() override
  {
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticVesselTree)

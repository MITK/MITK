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

#include "mitkPAComposedVolume.h"
#include "mitkIOUtil.h"
#include "mitkImageReadAccessor.h"
#include <string>

class mitkPhotoacousticComposedVolumeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticComposedVolumeTestSuite);
  MITK_TEST(TestCreateAndDestructComposedVolume);
  MITK_TEST(TestAccessInvalidFluenceComponent);
  MITK_TEST(TestAccessInvalidFluenceComponentIndex);
  MITK_TEST(TestAddMultiplePairs);
  MITK_TEST(TestSortFunctionality);
  MITK_TEST(TestAccessInvalidFluenceComponentForYOffset);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::pa::ComposedVolume::Pointer m_ComposedVolume;
  mitk::pa::TissueGeneratorParameters::Pointer m_DefaultParameters;
  mitk::pa::InSilicoTissueVolume::Pointer m_InSilicoTissueVolume;

public:

  void setUp() override
  {
    m_DefaultParameters = mitk::pa::TissueGeneratorParameters::New();
    m_DefaultParameters->SetXDim(5);
    m_DefaultParameters->SetYDim(5);
    m_DefaultParameters->SetZDim(5);
    m_InSilicoTissueVolume = mitk::pa::InSilicoTissueVolume::New(m_DefaultParameters);
    m_ComposedVolume = mitk::pa::ComposedVolume::New(m_InSilicoTissueVolume);
  }

  mitk::pa::FluenceYOffsetPair::Pointer createFluenceYOffsetPair(double value, double yOffset)
  {
    auto* data = new double[125];
    for (int i = 0; i < 125; ++i)
      data[i] = value;
    mitk::pa::Volume::Pointer volume = mitk::pa::Volume::New(data, 5, 5, 5);
    return mitk::pa::FluenceYOffsetPair::New(volume, yOffset);
  }

  void TestCreateAndDestructComposedVolume()
  {
    CPPUNIT_ASSERT(m_ComposedVolume->GetNumberOfFluenceComponents() == 0);
  }

  void TestAccessInvalidFluenceComponent()
  {
    bool caughtException = false;
    try
    {
      m_ComposedVolume->GetFluenceValue(0, 0, 0, 0);
    }
    catch (const mitk::Exception &)
    {
      caughtException = true;
    }
    CPPUNIT_ASSERT(caughtException);
  }

  void TestAddMultiplePairs()
  {
    m_ComposedVolume->AddSlice(createFluenceYOffsetPair(0, 0));
    CPPUNIT_ASSERT(m_ComposedVolume->GetNumberOfFluenceComponents() == 1);
    m_ComposedVolume->AddSlice(createFluenceYOffsetPair(1, 1));
    CPPUNIT_ASSERT(m_ComposedVolume->GetNumberOfFluenceComponents() == 2);
  }

  void TestSortFunctionality()
  {
    m_ComposedVolume->AddSlice(createFluenceYOffsetPair(2, 2));
    m_ComposedVolume->AddSlice(createFluenceYOffsetPair(-1, -1));
    m_ComposedVolume->AddSlice(createFluenceYOffsetPair(1, 1));
    m_ComposedVolume->AddSlice(createFluenceYOffsetPair(0, 0));
    m_ComposedVolume->AddSlice(createFluenceYOffsetPair(-2, -2));
    CPPUNIT_ASSERT(m_ComposedVolume->GetFluenceValue(0, 0, 2, 0) == 2);
    CPPUNIT_ASSERT(m_ComposedVolume->GetFluenceValue(1, 0, 2, 0) == -1);
    CPPUNIT_ASSERT(m_ComposedVolume->GetFluenceValue(2, 0, 2, 0) == 1);
    CPPUNIT_ASSERT(m_ComposedVolume->GetFluenceValue(3, 0, 2, 0) == 0);
    CPPUNIT_ASSERT(m_ComposedVolume->GetFluenceValue(4, 0, 2, 0) == -2);

    m_ComposedVolume->Sort();

    CPPUNIT_ASSERT(m_ComposedVolume->GetFluenceValue(0, 0, 2, 0) == -2);
    CPPUNIT_ASSERT(m_ComposedVolume->GetFluenceValue(1, 0, 2, 0) == -1);
    CPPUNIT_ASSERT(m_ComposedVolume->GetFluenceValue(2, 0, 2, 0) == 0);
    CPPUNIT_ASSERT(m_ComposedVolume->GetFluenceValue(3, 0, 2, 0) == 1);
    CPPUNIT_ASSERT(m_ComposedVolume->GetFluenceValue(4, 0, 2, 0) == 2);
  }

  void TestAccessInvalidFluenceComponentIndex()
  {
#ifdef _DEBUG
    m_ComposedVolume->AddSlice(createFluenceYOffsetPair(0, 0));
    bool caughtException = false;
    try
    {
      double unusedValue = m_ComposedVolume->GetFluenceValue(0, 1, 2, 300);
      unusedValue = 0;
    }
    catch (const mitk::Exception e)
    {
      caughtException = true;
    }
    CPPUNIT_ASSERT(caughtException);
#endif
  }

  void TestAccessInvalidFluenceComponentForYOffset()
  {
    bool caughtException = false;
    try
    {
      m_ComposedVolume->GetYOffsetForFluenceComponentInPixels(0);
    }
    catch (const mitk::Exception &)
    {
      caughtException = true;
    }
    CPPUNIT_ASSERT(caughtException);
  }

  void tearDown() override
  {
    m_ComposedVolume = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticComposedVolume)

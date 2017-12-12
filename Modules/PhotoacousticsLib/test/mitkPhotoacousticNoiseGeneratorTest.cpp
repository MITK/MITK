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

#include "mitkPAVolume.h"
#include "mitkPANoiseGenerator.h"

class mitkPhotoacousticNoiseGeneratorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticNoiseGeneratorTestSuite);
  MITK_TEST(testNoiseGenerator);
  CPPUNIT_TEST_SUITE_END();

private:

public:

  mitk::pa::Volume::Pointer m_Volume;

  void setUp() override
  {
  }

  void testNoiseGenerator()
  {
    int size = 1000 * 100 * 100;
    auto* volume = new double[size];
    for (int i = 0; i < size; i++)
    {
      volume[i] = 1;
    }
    m_Volume = mitk::pa::Volume::New(volume, 1000, 100, 100);
    mitk::pa::NoiseGenerator::ApplyNoiseModel(m_Volume, 0.75, 0.1);

    int negativecounter = 0;

    for (int i = 0; i < size; i++)
    {
      if (m_Volume->GetData()[i] <= 0)
      {
        negativecounter++;
      }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("More than one negative: " +
      std::to_string(negativecounter) + " (" +
      std::to_string((((double)negativecounter) / size) * 100) + "%)", negativecounter, 0);
  }

  void tearDown() override
  {
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticNoiseGenerator)

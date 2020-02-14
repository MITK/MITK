/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkImageReadAccessor.h>

#include "mitkPAInSilicoTissueVolume.h"
#include "mitkPATissueGenerator.h"
#include "mitkPAVesselMeanderStrategy.h"

class mitkPhotoacousticVesselMeanderStrategyTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticVesselMeanderStrategyTestSuite);
  MITK_TEST(TestCalculateNewPositionInStraightLine);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::pa::VesselMeanderStrategy::Pointer m_TestStrategy;
  mitk::pa::Vector::Pointer m_TestDirection;

public:

  void setUp() override
  {
    m_TestStrategy = mitk::pa::VesselMeanderStrategy::New();
    m_TestDirection = mitk::pa::Vector::New();
  }

  void TestCalculateNewPositionInStraightLine()
  {
    int d = 3;
    int e = 4;
    int f = 5;

    for (int i = -2; i <= 2; i++)
    {
      if (i == 0)
      {
        i++;
      }

      for (int j = -2; j <= 2; j++)
      {
        if (j == 0)
        {
          j++;
        }

        m_TestDirection->SetElement(0, d*j);
        m_TestDirection->SetElement(1, e*j);
        m_TestDirection->SetElement(2, f*j);

        mitk::pa::Vector::Pointer directionBefore = m_TestDirection->Clone();
        m_TestStrategy->CalculateNewDirectionVectorInStraightLine(m_TestDirection, 0, nullptr);
        CPPUNIT_ASSERT(mitk::pa::Equal(directionBefore, m_TestDirection, 1e-6, false));
      }
    }
  }

  void tearDown() override
  {
    m_TestStrategy = nullptr;
    m_TestDirection = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticVesselMeanderStrategy)

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
#include "mitkPATissueGenerator.h"
#include "mitkPAVesselMeanderStrategy.h"

class mitkPhotoacousticVesselMeanderStrategyTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticVesselMeanderStrategyTestSuite);
  MITK_TEST(TestCalculateNewPositionInStraightLine);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::pa::VesselMeanderStrategy::Pointer m_TestVector;
  mitk::pa::Vector::Pointer m_TestPostion;
  mitk::pa::Vector::Pointer m_TestDirection;

public:

  void setUp() override
  {
    m_TestVector = mitk::pa::VesselMeanderStrategy::New();
    m_TestPostion = mitk::pa::Vector::New();
    m_TestDirection = mitk::pa::Vector::New();
  }

  void TestCalculateNewPositionInStraightLine()
  {
    std::stringstream output;

    int a = 0;
    int b = 1;
    int c = 2;
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

        m_TestPostion->SetElement(0, a*i);
        m_TestPostion->SetElement(1, b*i);
        m_TestPostion->SetElement(2, c*i);
        m_TestDirection->SetElement(0, d*j);
        m_TestDirection->SetElement(1, e*j);
        m_TestDirection->SetElement(2, f*j);

        m_TestVector->CalculateNewPositionInStraightLine(m_TestPostion, m_TestDirection, 0, nullptr);

        MITK_INFO << "m_TestPosition Element(0) =" << m_TestPostion->GetElement(0);
        MITK_INFO << "Data0 =" << (a*i) + (d*j);
        MITK_INFO << "m_TestPosition Element(1) =" << m_TestPostion->GetElement(1);
        MITK_INFO << "Data1 =" << (b*i) + (e*j);
        MITK_INFO << "m_TestPosition Element(2) =" << m_TestPostion->GetElement(2);
        MITK_INFO << "Data2 =" << (c*i) + (f*j);

        output << "Element0 from m_TestPosition should be ";
        output << a*i + d*j;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(output.str(), true, a*i + d*j == m_TestPostion->GetElement(0));
        output.flush();

        output << "Element1 from m_TestPosition should be ";
        output << b*i + e*j;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(output.str(), true, b*i + e*j == m_TestPostion->GetElement(1));
        output.flush();

        output << "Element2 from m_TestPosition should be ";
        output << c*i + f*j;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(output.str(), true, c*i + f*j == m_TestPostion->GetElement(2));
        output.flush();
      }
    }
  }

  void tearDown() override
  {
    m_TestVector = nullptr;
    m_TestPostion = nullptr;
    m_TestDirection = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticVesselMeanderStrategy)

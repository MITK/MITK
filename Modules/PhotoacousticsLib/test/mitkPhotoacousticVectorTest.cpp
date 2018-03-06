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

#include "mitkPAVector.h"

class mitkPhotoacousticVectorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticVectorTestSuite);

  MITK_TEST(TestNormalizeVector);
  MITK_TEST(TestRotateVectorZeroDegrees);
  MITK_TEST(TestRotatedVectorPositiveDegrees);
  MITK_TEST(TestRotateVectorZeroDegrees);
  MITK_TEST(TestScaleVector);
  MITK_TEST(TestCloneVector);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::pa::Vector::Pointer m_TestVector;
  mitk::pa::Vector::Pointer m_TestReturnVector;

  const double DIF_VAL = 0.001;
  const double TWO_PI = 6.283185;

public:

  void setUp() override
  {
    m_TestVector = mitk::pa::Vector::New();
    m_TestReturnVector = mitk::pa::Vector::New();
  }

  void TestNormalizeVector()
  {
    std::stringstream output;
    int a = 2;
    int b = 3;
    int c = 4;

    m_TestVector->SetElement(0, a);
    m_TestVector->SetElement(1, b);
    m_TestVector->SetElement(2, c);

    output << "The vectorlength should be";
    output << sqrt(a*a + b*b + c*c);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(output.str(), sqrt(a*a + b*b + c*c), m_TestVector->GetNorm());
    output.flush();

    m_TestVector->Normalize();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The vectorlength should be 1.", true, m_TestVector->GetNorm() - 1 < DIF_VAL);
  }

  void TestRotateVectorZeroDegrees()
  {
    int a = 1;
    int b = 2;
    int c = 3;

    double length;

    m_TestVector->SetElement(0, a);
    m_TestVector->SetElement(1, b);
    m_TestVector->SetElement(2, c);

    length = m_TestVector->GetNorm();

    m_TestVector->Rotate(0, 0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector length should be equal", length, m_TestVector->GetNorm());

    CPPUNIT_ASSERT_MESSAGE("The vector value at index0 should be 1.0", m_TestVector->GetElement(0) - 1 < DIF_VAL);
    CPPUNIT_ASSERT_MESSAGE("The vector value at index1 should be 2.0", m_TestVector->GetElement(1) - 2 < DIF_VAL);
    CPPUNIT_ASSERT_MESSAGE("The vector value at index2 should be 3.0", m_TestVector->GetElement(2) - 3 < DIF_VAL);
  }

  void TestRotatedVectorPositiveDegrees()
  {
    MITK_INFO << atan2(0, 0);

    for (int r = 0; r < 10; r++)
    {
      for (double phi = 0.1; phi < 3; phi += 0.1)
      {
        for (double theta = 0.1; theta < 3; theta += 0.1)
        {
          double rotateTheta = 0.1;
          double rotatePhi = 0.1;

          m_TestVector->SetElement(0, r * sin(theta) * cos(phi));
          m_TestVector->SetElement(1, r * sin(theta) * sin(phi));
          m_TestVector->SetElement(2, r * cos(theta));

          m_TestVector->Rotate(rotateTheta, rotatePhi);

          double newTheta = fmod(theta + rotateTheta, TWO_PI);
          double newPhi = fmod(phi + rotatePhi, TWO_PI);

          double expectedX = r * sin(newTheta) * cos(newPhi);
          double expectedY = r * sin(newTheta) * sin(newPhi);
          double expectedZ = r * cos(newTheta);

          CPPUNIT_ASSERT_MESSAGE("The vector value at index0 should be " + std::to_string(expectedX) + " but was " + std::to_string(m_TestVector->GetElement(0))
            + " at r=" + std::to_string(r) + " phi=" + std::to_string(phi) + " theta=" + std::to_string(theta),
            m_TestVector->GetElement(0) - expectedX < DIF_VAL);
          CPPUNIT_ASSERT_MESSAGE("The vector value at index1 should be " + std::to_string(expectedY) + " but was " + std::to_string(m_TestVector->GetElement(0))
            + " at r=" + std::to_string(r) + " phi=" + std::to_string(phi) + " theta=" + std::to_string(theta),
            m_TestVector->GetElement(1) - expectedY < DIF_VAL);
          CPPUNIT_ASSERT_MESSAGE("The vector value at index2 should be " + std::to_string(expectedZ) + " but was " + std::to_string(m_TestVector->GetElement(0))
            + " at r=" + std::to_string(r) + " phi=" + std::to_string(phi) + " theta=" + std::to_string(theta),
            m_TestVector->GetElement(2) - expectedZ < DIF_VAL);
        }
      }
    }
  }

  void TestRotatedVectorNegativeDegrees()
  {
    for (int r = 0; r < 10; r++)
    {
      for (double phi = -0.1; phi > -3; phi -= 0.1)
      {
        for (double theta = -0.1; theta > -3; theta -= 0.1)
        {
          double rotateTheta = -0.1;
          double rotatePhi = -0.1;

          m_TestVector->SetElement(0, r * sin(theta) * cos(phi));
          m_TestVector->SetElement(1, r * sin(theta) * sin(phi));
          m_TestVector->SetElement(2, r * cos(theta));

          m_TestVector->Rotate(rotateTheta, rotatePhi);

          double newTheta = fmod(theta + rotateTheta, TWO_PI);
          double newPhi = fmod(phi + rotatePhi, TWO_PI);

          double expectedX = r * sin(newTheta) * cos(newPhi);
          double expectedY = r * sin(newTheta) * sin(newPhi);
          double expectedZ = r * cos(newTheta);

          CPPUNIT_ASSERT_MESSAGE("The vector value at index0 should be " + std::to_string(expectedX) + " but was " + std::to_string(m_TestVector->GetElement(0))
            + " at r=" + std::to_string(r) + " phi=" + std::to_string(phi) + " theta=" + std::to_string(theta),
            m_TestVector->GetElement(0) - expectedX < DIF_VAL);
          CPPUNIT_ASSERT_MESSAGE("The vector value at index1 should be " + std::to_string(expectedY) + " but was " + std::to_string(m_TestVector->GetElement(0))
            + " at r=" + std::to_string(r) + " phi=" + std::to_string(phi) + " theta=" + std::to_string(theta),
            m_TestVector->GetElement(1) - expectedY < DIF_VAL);
          CPPUNIT_ASSERT_MESSAGE("The vector value at index2 should be " + std::to_string(expectedZ) + " but was " + std::to_string(m_TestVector->GetElement(0))
            + " at r=" + std::to_string(r) + " phi=" + std::to_string(phi) + " theta=" + std::to_string(theta),
            m_TestVector->GetElement(2) - expectedZ < DIF_VAL);
        }
      }
    }
  }

  void TestScaleVector()
  {
    double a = 1.0;
    double b = 2.0;
    double c = 3.0;

    for (double testFactor = -2.0; testFactor <= 2.0; testFactor += 0.3)
    {
      double potElement0Fctr;
      double potElement1Fctr;
      double potElement2Fctr;

      std::stringstream output;

      m_TestVector->SetElement(0, a);
      m_TestVector->SetElement(1, b);
      m_TestVector->SetElement(2, c);

      potElement0Fctr = (m_TestVector->GetElement(0)*testFactor)*(m_TestVector->GetElement(0)*testFactor);
      potElement1Fctr = (m_TestVector->GetElement(1)*testFactor)*(m_TestVector->GetElement(1)*testFactor);
      potElement2Fctr = (m_TestVector->GetElement(2)*testFactor)*(m_TestVector->GetElement(2)*testFactor);

      m_TestVector->Scale(testFactor);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector length should not be equal",
        sqrt(potElement0Fctr + potElement1Fctr + potElement2Fctr), m_TestVector->GetNorm());

      output << "The vector value at index0 should be";
      output << a*testFactor;
      CPPUNIT_ASSERT_EQUAL_MESSAGE(output.str(), a*testFactor, m_TestVector->GetElement(0));
      output.flush();

      output << "The vector value at index1 should be";
      output << b*testFactor;
      CPPUNIT_ASSERT_EQUAL_MESSAGE(output.str(), b*testFactor, m_TestVector->GetElement(1));
      output.flush();

      output << "The vector value at index2 should be";
      output << c*testFactor;
      CPPUNIT_ASSERT_EQUAL_MESSAGE(output.str(), c*testFactor, m_TestVector->GetElement(2));
      output.flush();
    }
  }

  void TestCloneVector()
  {
    int a = 1;
    int b = 2;
    int c = 3;

    m_TestVector->SetElement(0, a);
    m_TestVector->SetElement(1, b);
    m_TestVector->SetElement(2, c);

    m_TestReturnVector = m_TestVector->Clone();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector length should be equal", (m_TestVector->GetNorm()), m_TestReturnVector->GetNorm());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector value at index0 should be equal", m_TestVector->GetElement(0), m_TestReturnVector->GetElement(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector value at index1 should be equal", m_TestVector->GetElement(1), m_TestReturnVector->GetElement(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector value at index2 should be equal", m_TestVector->GetElement(2), m_TestReturnVector->GetElement(2));

    m_TestReturnVector->Rotate(itk::Math::pi / 4, itk::Math::pi / 4);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector value at index0 should be not equal", true, m_TestVector->GetElement(0) != m_TestReturnVector->GetElement(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector value at index0 should be not equal", true, m_TestVector->GetElement(1) != m_TestReturnVector->GetElement(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector value at index0 should be not equal", true, m_TestVector->GetElement(2) != m_TestReturnVector->GetElement(2));

    for (double testFactor = -2.0; testFactor <= 2.0; testFactor += 0.3)
    {
      m_TestReturnVector->Scale(testFactor);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector value at index0 should be not equal", true, m_TestVector->GetElement(0) != m_TestReturnVector->GetElement(0));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector value at index0 should be not equal", true, m_TestVector->GetElement(1) != m_TestReturnVector->GetElement(1));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The vector value at index0 should be not equal", true, m_TestVector->GetElement(2) != m_TestReturnVector->GetElement(2));
    }
  }

  void tearDown() override
  {
    m_TestVector = nullptr;
    m_TestReturnVector = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticVector)

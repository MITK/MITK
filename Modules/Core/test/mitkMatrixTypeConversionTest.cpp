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

#include <string>

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include "mitkMatrix.h"


class mitkMatrixTypeConversionTestSuite : public mitk::TestFixture

{

  CPPUNIT_TEST_SUITE(mitkMatrixTypeConversionTestSuite);

  MITK_TEST(Mitk2Pod);
  MITK_TEST(Pod2Mitk);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::Matrix3D   mitkMatrix3D;
  mitk::ScalarType  podMatrix3D[3][3];


  /**
   * @brief Convenience method to test if one matrix has been assigned successfully to the other.
   *
   * More specifically, tests if m1 = m2 was performed correctly.
   *
   * @param m1    The matrix m1 of the assignment m1 = m2
   * @param m2    The matrix m2 of the assignment m1 = m2
   */
  template <typename T1, typename T2>
  void TestForEquality(const T1& m1, const T2& m2)
  {
    for (unsigned i = 0; i < 3; i++)
    {
      for (unsigned j = 0; j < 3; j++)
      {
        std::stringstream ss;
        ss << "element [" << i << "][" << j << "] equal for mitkMatrix and podMatrix";

        CPPUNIT_ASSERT_EQUAL_MESSAGE(ss.str(), true,
            (m1[i][j]==m2[i][j]));
      }
    }
  }


public:

  void setUp(void)
  {
    for (unsigned i = 0; i < 3; i++)
      for (unsigned j = 0; j < 3; j++)
      {
        mitkMatrix3D[i][j] = i + j;
        podMatrix3D[i][j]  = (mitk::ScalarType) (9 - (i + j));
      }
  }

  void tearDown(void)
  {

  }

  void Mitk2Pod(void)
  {
    mitkMatrix3D.ToArray(podMatrix3D);

    TestForEquality(mitkMatrix3D, podMatrix3D);
  }

  void Pod2Mitk(void)
  {
    mitkMatrix3D.FillMatrix(podMatrix3D);

    TestForEquality(podMatrix3D, mitkMatrix3D);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkMatrixTypeConversion)

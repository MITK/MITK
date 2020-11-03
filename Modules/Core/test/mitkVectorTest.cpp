/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
// Testing
#include "mitkTestFixture.h"
#include <mitkTestingMacros.h>

// std includes
#include <string>

// MITK includes
#include "mitkStringProperty.h"
#include <mitkNumericTypes.h>

// itksys
#include "itkImage.h"

// VTK includes
#include <vtkDebugLeaks.h>

// vnl includes
#include <vnl/vnl_vector_fixed.hxx>

class mitkVectorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkVectorTestSuite);

  MITK_TEST(ItkVecorEqualityUsingSameVector_Success);
  MITK_TEST(ItkVecorEqualityUsingDifferentVectors_Failure);
  MITK_TEST(ItkVecorEqualityForHigherEpsilonTolerance_Success);
  MITK_TEST(ItkVecorEqualityUsingDifferentVectorsWithElementWise_Success);

  MITK_TEST(ItkPointEqualitySamePoint_Success);
  MITK_TEST(ItkPointEqualityDifferentPoints_Failure);
  MITK_TEST(ItkPointEqualitForHigherEpsilons_Success);
  MITK_TEST(ItkPointEqualitDifferentPointsWithElementWise_Success);

  MITK_TEST(MitkVnlVectorEqualitySameMitkVnlVector_Success);
  MITK_TEST(MitkVnlVectorEqualityDifferentMitkVnlVectors_Failure);
  MITK_TEST(MitkVnlVectorEqualityHigherEpsilon_Success);
  MITK_TEST(MitkVnlVectorEqualityUsingDifferentMitkVnlVectorsWithElementWise_Success);

  MITK_TEST(VnlVectorEqualitySameVnlVector_Success);
  MITK_TEST(VnlVectorEqualityDifferentVnlVectors_Failure);
  MITK_TEST(VnlVectorEqualityDifferentVnlVectorsWithHighEps_Success);
  MITK_TEST(VnlVectorEqualityDifferentVnlVectorsWithLowEps_Success);
  MITK_TEST(VnlVectorEqualityDifferentVnlVectorsWithLowEps_Failure);

  MITK_TEST(ScalarEqualitySameScalar_Successs);
  MITK_TEST(ScalarEqualityDifferentScalarsDifferenceGreaterEps_Failure);
  MITK_TEST(ScalarEqualityDifferentScalarsDifferenceEqualEps_Successs);
  MITK_TEST(ScalarEqualityDifferentScalarsDifferenceLessEps_Successs);

  MITK_TEST(MatrixEqualitySameMatrixElementsWithEps_Success);
  MITK_TEST(MatrixEqualityElementWiseDifferentMatrixElementsWithEpsilonZero_Failure);
  MITK_TEST(MatrixEqualityDifferentMatrixElementsWithEpsilon_Success);
  MITK_TEST(MatrixEqualityRMSDifferentMatrixElementsWithEpsilon_Failure);
  MITK_TEST(MatrixEqualityRMSDifferentMatrixElementsWithEpsilonZero_Success);

  CPPUNIT_TEST_SUITE_END();

private:
  itk::Vector<mitk::ScalarType, 3> m_ItkVector_1;
  itk::Vector<mitk::ScalarType, 3> m_ItkVector_2;
  itk::Vector<mitk::ScalarType, 3> m_ItkVector_3;

  itk::Point<mitk::ScalarType, 3> m_ItkPoint_1;
  itk::Point<mitk::ScalarType, 3> m_ItkPoint_2;
  itk::Point<mitk::ScalarType, 3> m_ItkPoint_3;

  typedef mitk::ScalarType VnlValueType;
  vnl_vector_fixed<VnlValueType, 7> m_VnlVector_1;
  vnl_vector_fixed<VnlValueType, 7> m_VnlVector_2;
  vnl_vector_fixed<VnlValueType, 7> m_VnlVector_3;

  mitk::ScalarType m_Scalar1;
  mitk::ScalarType m_Scalar2;
  mitk::ScalarType m_Scalar3;
  mitk::ScalarType m_Scalar4;

  vnl_matrix_fixed<mitk::ScalarType, 3, 3> m_VnlMatrix3x3_1;
  vnl_matrix_fixed<mitk::ScalarType, 3, 3> m_VnlMatrix3x3_2;

  mitk::ScalarType m_Epsilon;

public:
  void setUp() override
  {
    m_ItkVector_1[0] = 4.6;
    m_ItkVector_1[1] = 9.76543;
    m_ItkVector_1[2] = 746.09;

    m_VnlVector_1[0] = 4.6;
    m_VnlVector_1[1] = 9.76543;
    m_VnlVector_1[2] = 746.09;
    m_VnlVector_1[3] = 56.98;
    m_VnlVector_1[4] = 22.32;
    m_VnlVector_1[5] = 1.00;
    m_VnlVector_1[6] = 746.09;

    m_Scalar1 = 0.5689;
    m_Scalar2 = m_Scalar1 + mitk::eps * 1.01;
    m_Scalar3 = m_Scalar1;
    m_Scalar4 = m_Scalar1 + mitk::eps * 0.95;

    m_VnlMatrix3x3_1(0, 0) = 1.1;
    m_VnlMatrix3x3_1(0, 1) = 0.4;
    m_VnlMatrix3x3_1(0, 2) = 5.3;
    m_VnlMatrix3x3_1(1, 0) = 2.7;
    m_VnlMatrix3x3_1(1, 1) = 3578.56418;
    m_VnlMatrix3x3_1(1, 2) = 123.56;
    m_VnlMatrix3x3_1(2, 0) = 546.89;
    m_VnlMatrix3x3_1(2, 1) = 0.0001;
    m_VnlMatrix3x3_1(2, 2) = 1.0;

    m_VnlMatrix3x3_2(0, 0) = 1.1000009;
    m_VnlMatrix3x3_2(0, 1) = 0.4000009;
    m_VnlMatrix3x3_2(0, 2) = 5.3000009;
    m_VnlMatrix3x3_2(1, 0) = 2.7000009;
    m_VnlMatrix3x3_2(1, 1) = 3578.5641809;
    m_VnlMatrix3x3_2(1, 2) = 123.5600009;
    m_VnlMatrix3x3_2(2, 0) = 546.8900009;
    m_VnlMatrix3x3_2(2, 1) = 0.0001009;
    m_VnlMatrix3x3_2(2, 2) = 1.0000009;

    m_Epsilon = 0.000001;
  }

  void tearDown() override
  {
    m_ItkVector_1.Fill(0);
    m_ItkVector_2.Fill(0);
    m_ItkVector_3.Fill(0);
    m_ItkPoint_1.Fill(0);
    m_ItkPoint_2.Fill(0);
    m_ItkPoint_3.Fill(0);

    m_VnlVector_1.fill(0);
    m_VnlVector_2.fill(0);
    m_VnlVector_3.fill(0);

    m_Scalar1 = 0;
    m_Scalar2 = 0;
    m_Scalar3 = 0;
    m_Scalar4 = 0;

    m_VnlMatrix3x3_1.fill(0);
    m_VnlMatrix3x3_2.fill(0);
  }

  void ItkVecorEqualityUsingSameVector_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Test vector equality using the same vector with mitk::eps",
                           mitk::Equal(m_ItkVector_1, m_ItkVector_1));
  }

  void ItkVecorEqualityUsingDifferentVectors_Failure()
  {
    for (int i = 0; i < 3; i++)
    {
      m_ItkVector_2[i] = m_ItkVector_1[i] - mitk::eps * 1.1;
    }
    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Test vector equality using different vectors with an element-wise difference greater than mitk::eps",
      mitk::Equal(m_ItkVector_1, m_ItkVector_2));
  }

  void ItkVecorEqualityForHigherEpsilonTolerance_Success()
  {
    for (int i = 0; i < 3; i++)
    {
      m_ItkVector_2[i] = m_ItkVector_1[i] - mitk::eps * 1.1;
    }
    CPPUNIT_ASSERT_MESSAGE("Vectors are equal for higher epsilon tolerance ( 1.2 * mitk::eps )",
                           mitk::Equal(m_ItkVector_1, m_ItkVector_2, mitk::eps * 1.2));
  }

  void ItkVecorEqualityUsingDifferentVectorsWithElementWise_Success()
  {
    for (int i = 0; i < 3; i++)
    {
      m_ItkVector_3[i] = m_ItkVector_1[i] - mitk::eps * 0.9;
    }
    CPPUNIT_ASSERT_MESSAGE(
      "Test vector equality using different vectors with an element-wise difference less than mitk::eps",
      mitk::Equal(m_ItkVector_1, m_ItkVector_3));
  }

  void ItkPointEqualitySamePoint_Success()
  {
    // test itk point equality methods
    for (int i = 0; i < 3; i++)
    {
      m_ItkPoint_1[i] = m_ItkVector_1[i];
    }
    CPPUNIT_ASSERT_MESSAGE("Test point equality using the same point with mitk::eps",
                           mitk::Equal(m_ItkPoint_1, m_ItkPoint_1));
  }

  void ItkPointEqualityDifferentPoints_Failure()
  {
    for (int i = 0; i < 3; i++)
    {
      m_ItkPoint_1[i] = m_ItkVector_1[i];
      m_ItkPoint_2[i] = m_ItkVector_2[i];
    }
    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Test point equality using different points with an element-wise difference greater than mitk::eps",
      mitk::Equal(m_ItkPoint_1, m_ItkPoint_2));
  }

  void ItkPointEqualitForHigherEpsilons_Success()
  {
    for (int i = 0; i < 3; i++)
    {
      m_ItkVector_2[i] = m_ItkVector_1[i] - mitk::eps * 1.1;
    }

    for (int i = 0; i < 3; i++)
    {
      m_ItkPoint_1[i] = m_ItkVector_1[i];
      m_ItkPoint_2[i] = m_ItkVector_2[i];
    }
    CPPUNIT_ASSERT_MESSAGE("Points are equal for higher epsilon tolerance ( 1.2 * mitk::eps )",
                           mitk::Equal(m_ItkPoint_1, m_ItkPoint_2, mitk::eps * 1.2));
  }

  void ItkPointEqualitDifferentPointsWithElementWise_Success()
  {
    for (int i = 0; i < 3; i++)
    {
      m_ItkVector_3[i] = m_ItkVector_1[i] - mitk::eps * 0.9;
    }

    for (int i = 0; i < 3; i++)
    {
      m_ItkPoint_1[i] = m_ItkVector_1[i];
      m_ItkPoint_3[i] = m_ItkVector_3[i];
    }
    CPPUNIT_ASSERT_MESSAGE(
      "Test point equality using different points with an element-wise difference less than mitk::eps",
      mitk::Equal(m_ItkPoint_1, m_ItkPoint_3));
  }

  void MitkVnlVectorEqualitySameMitkVnlVector_Success()
  {
    mitk::VnlVector mitkVnlVector_1(3);

    for (int i = 0; i < 3; i++)
    {
      mitkVnlVector_1.put(i, m_ItkVector_1[i]);
    }

    CPPUNIT_ASSERT_MESSAGE("Test mitk vnl vector equality using the same mitk vnl vector with mitk::eps",
                           mitk::Equal(mitkVnlVector_1, mitkVnlVector_1));
  }

  void MitkVnlVectorEqualityDifferentMitkVnlVectors_Failure()
  {
    mitk::VnlVector mitkVnlVector_1(3);
    mitk::VnlVector mitkVnlVector_2(3);

    for (int i = 0; i < 3; i++)
    {
      mitkVnlVector_1.put(i, m_ItkVector_1[i]);
      mitkVnlVector_2.put(i, m_ItkVector_2[i]);
    }

    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Test mitk vnl vector equality using different mitk vnl vectors with an element-wise difference "
      "greater than mitk::eps",
      mitk::Equal(mitkVnlVector_1, mitkVnlVector_2));
  }

  void MitkVnlVectorEqualityHigherEpsilon_Success()
  {
    mitk::VnlVector mitkVnlVector_1(3);
    mitk::VnlVector mitkVnlVector_2(3);

    for (int i = 0; i < 3; i++)
    {
      m_ItkVector_2[i] = m_ItkVector_1[i] - mitk::eps * 1.1;
    }

    for (int i = 0; i < 3; i++)
    {
      mitkVnlVector_1.put(i, m_ItkVector_1[i]);
      mitkVnlVector_2.put(i, m_ItkVector_2[i]);
    }

    CPPUNIT_ASSERT_MESSAGE("Vnl vectors are equal for higher epsilon tolerance ( 1.2 * mitk::eps )",
                           mitk::Equal(mitkVnlVector_1, mitkVnlVector_2, mitk::eps * 1.2));
  }

  void MitkVnlVectorEqualityUsingDifferentMitkVnlVectorsWithElementWise_Success()
  {
    mitk::VnlVector mitkVnlVector_1(3);
    mitk::VnlVector mitkVnlVector_3(3);

    for (int i = 0; i < 3; i++)
    {
      m_ItkVector_3[i] = m_ItkVector_1[i] - mitk::eps * 0.9;
    }

    for (int i = 0; i < 3; i++)
    {
      mitkVnlVector_1.put(i, m_ItkVector_1[i]);
      mitkVnlVector_3.put(i, m_ItkVector_3[i]);
    }

    CPPUNIT_ASSERT_MESSAGE("Test mitk vnl vector equality using "
                           "different mitk vnl vectors with an "
                           "element-wise difference less than mitk::eps",
                           mitk::Equal(mitkVnlVector_1, mitkVnlVector_3));
  }

  void VnlVectorEqualitySameVnlVector_Success()
  {
    // test vnl_vector equality method

    CPPUNIT_ASSERT_MESSAGE("vnl_fixed : v_1 == v_1 ", (mitk::Equal<VnlValueType, 7>(m_VnlVector_1, m_VnlVector_1)));
  }

  // the v_2 is constructed so that the equality test fails for mitk::eps, the norm of the difference between the
  // vectors is 7 * eps/6.9
  void VnlVectorEqualityDifferentVnlVectors_Failure()
  {
    for (int i = 0; i < 7; i++)
    {
      m_VnlVector_2[i] = m_VnlVector_1[i] - mitk::eps * 1.1f;
    }

    CPPUNIT_ASSERT_NO_THROW_MESSAGE("vnl_fixed : v_1 != v_2 with mitk::eps ",
                                    (mitk::Equal<VnlValueType, 7>(m_VnlVector_1, m_VnlVector_2)));
  }

  // increase the epsilon value used for testing equality - should now pass  ( 1.2 * mitk::eps > 7 * mitk::eps/6.9 )
  void VnlVectorEqualityDifferentVnlVectorsWithHighEps_Success()
  {
    for (int i = 0; i < 7; i++)
    {
      m_VnlVector_2[i] = m_VnlVector_1[i] - mitk::eps * 1.1f;
    }

    CPPUNIT_ASSERT_MESSAGE("vnl_fixed : v_1 == v_2 with eps = 1.2 * mitk::eps ",
                           (mitk::Equal<VnlValueType, 7>(m_VnlVector_1, m_VnlVector_2, mitk::eps * 1.2f)));
  }

  void VnlVectorEqualityDifferentVnlVectorsWithLowEps_Success()
  {
    for (int i = 0; i < 7; i++)
    {
      m_VnlVector_3[i] = m_VnlVector_1[i] - mitk::eps * 0.9f;
    }

    CPPUNIT_ASSERT_MESSAGE("vnl_fixed : v_1 == v_3 with eps = 0.8 * mitk::eps ",
                           (mitk::Equal<VnlValueType, 7>(m_VnlVector_1, m_VnlVector_3, mitk::eps)));
  }

  void VnlVectorEqualityDifferentVnlVectorsWithLowEps_Failure()
  {
    for (int i = 0; i < 7; i++)
    {
      m_VnlVector_3[i] = m_VnlVector_1[i] - mitk::eps * 0.9f;
    }

    CPPUNIT_ASSERT_NO_THROW_MESSAGE("vnl_fixed : v_1 != v_3 with eps = 0.8 * mitk::eps ",
                                    (mitk::Equal<VnlValueType, 7>(m_VnlVector_1, m_VnlVector_3, mitk::eps * 0.8f)));
  }

  void ScalarEqualitySameScalar_Successs()
  {
    // test scalar equality method
    CPPUNIT_ASSERT_MESSAGE("Test scalar equality using the same scalar with mitk::eps",
                           mitk::Equal(m_Scalar1, m_Scalar1));
  }
  void ScalarEqualityDifferentScalarsDifferenceGreaterEps_Failure()
  {
    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Test scalar equality using the different scalars with a difference greater than mitk::eps",
      mitk::Equal(m_Scalar1, m_Scalar2));
  }

  void ScalarEqualityDifferentScalarsDifferenceEqualEps_Successs()
  {
    CPPUNIT_ASSERT_MESSAGE("Test scalar equality using the different scalars with a difference equal to mitk::eps",
                           mitk::Equal(m_Scalar1, m_Scalar3));
  }

  void ScalarEqualityDifferentScalarsDifferenceLessEps_Successs()
  {
    CPPUNIT_ASSERT_MESSAGE("Test scalar equality using the different scalars with a difference less than mitk::eps",
                           mitk::Equal(m_Scalar1, m_Scalar4));
  }

  void MatrixEqualitySameMatrixElementsWithEps_Success()
  {
    // test matrix equality methods
    CPPUNIT_ASSERT_MESSAGE("Test for matrix equality with given epsilon=mitk::eps and exactly the same matrix elements",
                           mitk::MatrixEqualElementWise(m_VnlMatrix3x3_1, m_VnlMatrix3x3_1, mitk::eps));
  }

  void MatrixEqualityElementWiseDifferentMatrixElementsWithEpsilonZero_Failure()
  {
    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Test for matrix equality with given epsilon=0.0 and slightly different matrix elements",
      mitk::MatrixEqualElementWise(m_VnlMatrix3x3_1, m_VnlMatrix3x3_2, 0.0));
  }

  void MatrixEqualityDifferentMatrixElementsWithEpsilon_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Test for matrix equality with given epsilon and slightly different matrix elements",
                           mitk::MatrixEqualElementWise(m_VnlMatrix3x3_1, m_VnlMatrix3x3_2, m_Epsilon));
  }

  void MatrixEqualityRMSDifferentMatrixElementsWithEpsilon_Failure()
  {
    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Test for matrix equality with given epsilon=0.0 and slightly different matrix elements",
      mitk::MatrixEqualRMS(m_VnlMatrix3x3_1, m_VnlMatrix3x3_2, 0.0));
  }

  void MatrixEqualityRMSDifferentMatrixElementsWithEpsilonZero_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Test for matrix equality with given epsilon and slightly different matrix elements",
                           mitk::MatrixEqualRMS(m_VnlMatrix3x3_1, m_VnlMatrix3x3_2, m_Epsilon));
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkVector)

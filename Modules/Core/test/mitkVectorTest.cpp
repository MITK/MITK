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
// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
#include <mitkTestingMacros.h>

// std includes
#include <string>

// MITK includes
#include <mitkNumericTypes.h>
#include "mitkStringProperty.h"

//itksys
#include "itkImage.h"

// VTK includes
#include <vtkDebugLeaks.h>

//vnl includes
#include <vnl/vnl_vector_fixed.hxx>

class mitkVectorTestSuite : public mitk::TestFixture
{
	CPPUNIT_TEST_SUITE(mitkVectorTestSuite);

	MITK_TEST(TestingItkVecorEqualityMethods_Success);
    MITK_TEST(TestingItkPointEqualityMethods_Failure);
    MITK_TEST(TestingMitkVnlVectorEqualityMethods_Failure);
	MITK_TEST(TestingVnl_VectorEqualityMethod_Success);
	MITK_TEST(TestingScalarEqualityMethod_Success);
	MITK_TEST(TestingMatrixEqualityMethods_Success);

	CPPUNIT_TEST_SUITE_END();

private:
	itk::Vector<mitk::ScalarType, 3> m_itkVector_1;
	itk::Vector<mitk::ScalarType, 3> m_itkVector_2;
	itk::Vector<mitk::ScalarType, 3> m_itkVector_3;

public:
	void setUp() 
	{
		itk::Vector<mitk::ScalarType, 3> m_itkVector_1;
		itk::Vector<mitk::ScalarType, 3> m_itkVector_2;
		itk::Vector<mitk::ScalarType, 3> m_itkVector_3;

		m_itkVector_1[0] = 4.6;
		m_itkVector_1[1] = 9.76543;
		m_itkVector_1[2] = 746.09;

	}
	void tearDown() 
	{
		m_itkVector_1[0] = 0;
		m_itkVector_1[1] = 0;
		m_itkVector_1[2] = 0;
	}

	void TestingItkVecorEqualityMethods_Success(){
	// test itk vector equality methods

	  for (int i = 0; i < 3; i++)
	  {
		  m_itkVector_2[i] = m_itkVector_1[i] - mitk::eps * 1.1;
		  m_itkVector_3[i] = m_itkVector_1[i] - mitk::eps * 0.9;
	  }

	  CPPUNIT_ASSERT_MESSAGE("Test vector equality using the same vector with mitk::eps", mitk::Equal(m_itkVector_1, m_itkVector_1));
	  CPPUNIT_ASSERT_MESSAGE("Test vector equality using different vectors with an element-wise difference greater than mitk::eps", 
		!mitk::Equal(m_itkVector_1, m_itkVector_2));
	  CPPUNIT_ASSERT_MESSAGE("Vectors are equal for higher epsilon tolerance ( 1.2 * mitk::eps )", mitk::Equal(m_itkVector_1, m_itkVector_2, mitk::eps * 1.2));
	  CPPUNIT_ASSERT_MESSAGE("Test vector equality using different vectors with an element-wise difference less than mitk::eps", 
		  mitk::Equal(m_itkVector_1, m_itkVector_3));
	}

	void TestingItkPointEqualityMethods_Failure() {
		// test itk point equality methods
		itk::Point<mitk::ScalarType, 3> itkPoint_1;
		itk::Point<mitk::ScalarType, 3> itkPoint_2;
		itk::Point<mitk::ScalarType, 3> itkPoint_3;
		for (int i = 0; i < 3; i++)
		{
			itkPoint_1[i] = m_itkVector_1[i];
			itkPoint_2[i] = m_itkVector_2[i];
			itkPoint_3[i] = m_itkVector_3[i];
		}
		CPPUNIT_ASSERT_MESSAGE("Test point equality using the same point with mitk::eps", mitk::Equal(itkPoint_1, itkPoint_1));
		CPPUNIT_ASSERT_MESSAGE("Test point equality using different points with an element-wise difference greater than mitk::eps", 
		mitk::Equal(itkPoint_1, itkPoint_2));
		CPPUNIT_ASSERT_MESSAGE("Points are equal for higher epsilon tolerance ( 1.2 * mitk::eps )", mitk::Equal(itkPoint_1, itkPoint_2, mitk::eps * 1.2));
		CPPUNIT_ASSERT_MESSAGE("Test point equality using different points with an element-wise difference less than mitk::eps", 
		mitk::Equal(itkPoint_1, itkPoint_3));
	}

	void TestingMitkVnlVectorEqualityMethods_Failure() {
		// test mitk vnl vector equality methods
		mitk::VnlVector mitk_vnl_vector_1(3);
		mitk::VnlVector mitk_vnl_vector_2(3);
		mitk::VnlVector mitk_vnl_vector_3(3);
		for (int i = 0; i < 3; i++)
		{
			mitk_vnl_vector_1.put(i, m_itkVector_1[i]);
			mitk_vnl_vector_2.put(i, m_itkVector_2[i]);
			mitk_vnl_vector_3.put(i, m_itkVector_1[i]);
		}

		CPPUNIT_ASSERT_MESSAGE("Test mitk vnl vector equality using the same mitk vnl vector with mitk::eps",
		  mitk::Equal(mitk_vnl_vector_1, mitk_vnl_vector_1));
		CPPUNIT_ASSERT_MESSAGE("Test mitk vnl vector equality using different mitk vnl vectors with an element-wise difference " "greater than mitk::eps", 
		mitk::Equal(mitk_vnl_vector_1, mitk_vnl_vector_2));
		CPPUNIT_ASSERT_MESSAGE("Vnl vectors are equal for higher epsilon tolerance ( 1.2 * mitk::eps )", mitk::Equal(mitk_vnl_vector_1, mitk_vnl_vector_2, mitk::eps * 1.2));
		CPPUNIT_ASSERT_MESSAGE("Test mitk vnl vector equality using " "different mitk vnl vectors with an "	"element-wise difference less than mitk::eps", 
		mitk::Equal(mitk_vnl_vector_1, mitk_vnl_vector_3));
	}

	void TestingVnl_VectorEqualityMethod_Success() {
		// test vnl_vector equality method
		typedef mitk::ScalarType VnlValueType;
		vnl_vector_fixed<VnlValueType, 7> vnlVector_1;
		vnlVector_1[3] = 56.98;
		vnlVector_1[4] = 22.32;
		vnlVector_1[5] = 1.00;
		vnlVector_1[6] = 746.09;
		vnl_vector_fixed<VnlValueType, 7> vnlVector_2;
		vnl_vector_fixed<VnlValueType, 7> vnlVector_3;
		for (int i = 0; i < 7; i++)
		{
			if (i < 3)
			{
				vnlVector_1.put(i, m_itkVector_1[i]);
			}

			vnlVector_2[i] = vnlVector_1[i] - mitk::eps * 1.1f;
			vnlVector_3[i] = vnlVector_1[i] - mitk::eps * 0.9f;
		}

		CPPUNIT_ASSERT_MESSAGE("vnl_fixed : v_1 == v_1 ", (mitk::Equal<VnlValueType, 7>(vnlVector_1, vnlVector_1)));
		// the v_2 is constructed so that the equality test fails for mitk::eps, the norm of the difference between the
		// vectors is 7 * eps/6.9
		CPPUNIT_ASSERT_MESSAGE("vnl_fixed : v_1 != v_2 with mitk::eps ", !(mitk::Equal<VnlValueType, 7>(vnlVector_1, vnlVector_2)));
		// increase the epsilon value used for testing equality - should now pass  ( 1.2 * mitk::eps > 7 * mitk::eps/6.9 )
		CPPUNIT_ASSERT_MESSAGE("vnl_fixed : v_1 == v_2 with eps = 1.2 * mitk::eps ", (mitk::Equal<VnlValueType, 7>(vnlVector_1, vnlVector_2, mitk::eps * 1.2f)));
		CPPUNIT_ASSERT_MESSAGE("vnl_fixed : v_1 == v_3 with eps = 0.8 * mitk::eps ", (mitk::Equal<VnlValueType, 7>(vnlVector_1, vnlVector_3, mitk::eps)));
		CPPUNIT_ASSERT_MESSAGE("vnl_fixed : v_1 != v_3 with eps = 0.8 * mitk::eps ", !(mitk::Equal<VnlValueType, 7>(vnlVector_1, vnlVector_3, mitk::eps * 0.8f)));
	}

	void TestingScalarEqualityMethod_Success() {
		// test scalar equality method
		mitk::ScalarType scalar1 = 0.5689;
		mitk::ScalarType scalar2 = scalar1 + mitk::eps * 1.01;
		mitk::ScalarType scalar3 = scalar1;
		mitk::ScalarType scalar4 = scalar1 + mitk::eps * 0.95;
		CPPUNIT_ASSERT_MESSAGE("Test scalar equality using the same scalar with mitk::eps", mitk::Equal(scalar1, scalar1));
		CPPUNIT_ASSERT_MESSAGE("Test scalar equality using the different scalars with a difference greater than mitk::eps", !mitk::Equal(scalar1, scalar2));
		CPPUNIT_ASSERT_MESSAGE("Test scalar equality using the different scalars with a difference equal to mitk::eps", mitk::Equal(scalar1, scalar3));
		CPPUNIT_ASSERT_MESSAGE("Test scalar equality using the different scalars with a difference less than mitk::eps", mitk::Equal(scalar1, scalar4));
	}

	void TestingMatrixEqualityMethods_Success() {
		// test matrix equality methods
		vnl_matrix_fixed<mitk::ScalarType, 3, 3> vnlMatrix3x3_1;
		vnlMatrix3x3_1(0, 0) = 1.1;
		vnlMatrix3x3_1(0, 1) = 0.4;
		vnlMatrix3x3_1(0, 2) = 5.3;
		vnlMatrix3x3_1(1, 0) = 2.7;
		vnlMatrix3x3_1(1, 1) = 3578.56418;
		vnlMatrix3x3_1(1, 2) = 123.56;
		vnlMatrix3x3_1(2, 0) = 546.89;
		vnlMatrix3x3_1(2, 1) = 0.0001;
		vnlMatrix3x3_1(2, 2) = 1.0;
		vnl_matrix_fixed<mitk::ScalarType, 3, 3> vnlMatrix3x3_2;
		vnlMatrix3x3_2(0, 0) = 1.1000009;
		vnlMatrix3x3_2(0, 1) = 0.4000009;
		vnlMatrix3x3_2(0, 2) = 5.3000009;
		vnlMatrix3x3_2(1, 0) = 2.7000009;
		vnlMatrix3x3_2(1, 1) = 3578.5641809;
		vnlMatrix3x3_2(1, 2) = 123.5600009;
		vnlMatrix3x3_2(2, 0) = 546.8900009;
		vnlMatrix3x3_2(2, 1) = 0.0001009;
		vnlMatrix3x3_2(2, 2) = 1.0000009;

		mitk::ScalarType epsilon = 0.000001;
		CPPUNIT_ASSERT_MESSAGE("Test for matrix equality with given epsilon=mitk::eps and exactly the same matrix elements",
			mitk::MatrixEqualElementWise(vnlMatrix3x3_1, vnlMatrix3x3_1, mitk::eps));
		CPPUNIT_ASSERT_MESSAGE("Test for matrix equality with given epsilon=0.0 and slightly different matrix elements",
			!mitk::MatrixEqualElementWise(vnlMatrix3x3_1, vnlMatrix3x3_2, 0.0));
		CPPUNIT_ASSERT_MESSAGE("Test for matrix equality with given epsilon and slightly different matrix elements",
			mitk::MatrixEqualElementWise(vnlMatrix3x3_1, vnlMatrix3x3_2, epsilon));
		CPPUNIT_ASSERT_MESSAGE("Test for matrix equality with given epsilon=0.0 and slightly different matrix elements",
			!mitk::MatrixEqualRMS(vnlMatrix3x3_1, vnlMatrix3x3_2, 0.0));
		CPPUNIT_ASSERT_MESSAGE("Test for matrix equality with given epsilon and slightly different matrix elements",
			mitk::MatrixEqualRMS(vnlMatrix3x3_1, vnlMatrix3x3_2, epsilon));
	}
};
MITK_TEST_SUITE_REGISTRATION(mitkVector)


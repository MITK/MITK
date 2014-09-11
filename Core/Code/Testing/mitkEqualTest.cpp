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

#include "mitkTestingMacros.h"
#include <mitkTestFixture.h>
#include <mitkNumericTypes.h>

class mitkEqualTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkEqualTestSuite);
  MITK_TEST(Equal_SameMitkVector3D_ReturnTrue);
  MITK_TEST(Equal_SameMitkVector2D_ReturnTrue);
  MITK_TEST(Equal_DifferentMitkVector3D_ReturnFalse);
  MITK_TEST(Equal_DifferentMitkVector2D_ReturnFalse);
  MITK_TEST(Equal_SameMitkVnlVector_ReturnTrue);
  MITK_TEST(Equal_DifferentMitkVnlVector_ReturnFalse);
  MITK_TEST(Equal_SameVnlVector_ReturnTrue);
  MITK_TEST(Equal_DifferentVnlVector_ReturnFalse);
  MITK_TEST(Equal_SameItkVector_ReturnTrue);
  MITK_TEST(Equal_DifferentItkVector_ReturnFalse);
  MITK_TEST(Equal_SameScalar_ReturnTrue);
  MITK_TEST(Equal_DifferentScalar_ReturnFalse);
  MITK_TEST(Equal_SameItkPoint_ReturnTrue);
  MITK_TEST(Equal_DifferentItkPoint_ReturnFalse);
  CPPUNIT_TEST_SUITE_END();

public:

  void Equal_SameMitkVector3D_ReturnTrue()
  {
    mitk::Vector3D v1;
    v1.Fill(1);
    mitk::Vector3D v2;
    v2.Fill(1);
    CPPUNIT_ASSERT(mitk::Equal(v1,v2));
  }

  void Equal_SameMitkVector2D_ReturnTrue()
  {
    mitk::Vector2D v1;
    v1.Fill(2);
    mitk::Vector2D v2;
    v2.Fill(2);
    CPPUNIT_ASSERT(mitk::Equal(v1,v2));
  }

  void Equal_DifferentMitkVector3D_ReturnFalse()
  {
    mitk::Vector3D v1;
    v1.SetElement(0, 1);
    v1.SetElement(1, 1);
    v1.SetElement(2, 1);
    mitk::Vector3D v2;
    v2.SetElement(0, 2);
    v2.SetElement(1, 1);
    v2.SetElement(2, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Coparison of 2 different 3D MITK vectors (first element). Result should be false.", mitk::Equal(v1,v2), false);
    mitk::Vector3D v3;
    v3.SetElement(0, 1);
    v3.SetElement(1, 2);
    v3.SetElement(2, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Coparison of 2 different 3D MITK vectors (second element). Result should be false.", mitk::Equal(v1,v3), false);
    mitk::Vector3D v4;
    v4.SetElement(0, 1);
    v4.SetElement(1, 1);
    v4.SetElement(2, 2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Coparison of 2 different 3D MITK vectors (third element). Result should be false.", mitk::Equal(v1,v4), false);
  }

  void Equal_DifferentMitkVector2D_ReturnFalse()
  {
    mitk::Vector2D v1;
    v1.SetElement(0, 1);
    v1.SetElement(1, 1);
    mitk::Vector2D v2;
    v2.SetElement(0, 2);
    v2.SetElement(1, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Coparison of 2 different 2D MITK vectors (first element). Result should be false.", mitk::Equal(v1,v2), false);
    mitk::Vector2D v3;
    v3.SetElement(0, 1);
    v3.SetElement(1, 2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Coparison of 2 different 2D MITK vectors (second element). Result should be false.", mitk::Equal(v1,v3), false);
  }

  void Equal_SameMitkVnlVector_ReturnTrue()
  {
    mitk::VnlVector v1(2);
    v1.fill(8);
    mitk::VnlVector v2(2);
    v2.fill(8);
    CPPUNIT_ASSERT(mitk::Equal(v1,v2));
  }

  void Equal_DifferentMitkVnlVector_ReturnFalse()
  {
    mitk::VnlVector v1(3);
    v1.fill(9.2);
    mitk::VnlVector v2(3);
    v2.fill(9.2);
    v2[0] = 6;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Comparison of 2 different MITK VNL vectors. Result should be false.", mitk::Equal(v1,v2), false);
  }

  void Equal_SameVnlVector_ReturnTrue()
  {
    vnl_vector_fixed<mitk::ScalarType, 4u> v1;
    v1.fill(3.0);
    vnl_vector_fixed<mitk::ScalarType, 4u> v2;
    v2.fill(3.0);
    CPPUNIT_ASSERT(mitk::Equal(v1,v2));
  }

  void Equal_DifferentVnlVector_ReturnFalse()
  {
    vnl_vector_fixed<mitk::ScalarType, 2u> v1;
    v1.fill(8.231);
    vnl_vector_fixed<mitk::ScalarType, 2u> v2;
    v2[0] = 3.1;
    v2[1] = 8.231;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Comparison of 2 different VNL vectors (first element). Result should be false.", mitk::Equal(v1,v2), false);
    vnl_vector_fixed<mitk::ScalarType, 2u> v3;
    v3[0] = 8.231;
    v3[1] = 2.14;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Comparison of 2 different VNL vectors (first element). Result should be false.", mitk::Equal(v1,v3), false);
  }

  void Equal_SameItkVector_ReturnTrue()
  {
    itk::Vector<mitk::ScalarType, 3u> v1;
    v1.Fill(1.32);
    itk::Vector<mitk::ScalarType, 3u> v2;
    v2.Fill(1.32);
    mitk::Equal(v1, v2);
    CPPUNIT_ASSERT(mitk::Equal(v1,v2));
  }

  void Equal_DifferentItkVector_ReturnFalse()
  {
    itk::Vector<mitk::ScalarType, 3u> v1;
    v1.Fill(5.2);
    itk::Vector<mitk::ScalarType, 3u> v2;
    v2.Fill(5.2);
    v2.SetElement(0, 1.4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Coparison of 2 different ITK vectors (first element). Result should be false.", mitk::Equal(v1,v2), false);
    itk::Vector<mitk::ScalarType, 3u> v3;
    v3.Fill(5.2);
    v3.SetElement(1, 1.4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Coparison of 2 different ITK vectors (second element). Result should be false.", mitk::Equal(v1,v3), false);
    itk::Vector<mitk::ScalarType, 3u> v4;
    v4.Fill(5.2);
    v4.SetElement(2, 1.4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Coparison of 2 different ITK vectors (third element). Result should be false.", mitk::Equal(v1,v4), false);
  }

  void Equal_SameScalar_ReturnTrue()
  {
    mitk::ScalarType a = 6.432;
    mitk::ScalarType b = 6.432;
    CPPUNIT_ASSERT(mitk::Equal(a,b));
  }

  void Equal_DifferentScalar_ReturnFalse()
  {
    mitk::ScalarType a = 0.6;
    mitk::ScalarType b = 0.6 + 1.01*mitk::eps;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Comparison of 2 different scalars. Result should be false", mitk::Equal(a,b), false);
  }

  void Equal_SameItkPoint_ReturnTrue()
  {
    itk::Point<mitk::ScalarType, 2> p1;
    p1.Fill(3.21);
    itk::Point<mitk::ScalarType, 2> p2;
    p2.Fill(3.21);
    CPPUNIT_ASSERT(mitk::Equal(p1,p2));
  }

  void Equal_DifferentItkPoint_ReturnFalse()
  {
    itk::Point<mitk::ScalarType, 2> p1;
    p1.Fill(2.1);
    itk::Point<mitk::ScalarType, 2> p2;
    p2[0] = 1;
    p2[1] = 2.1;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Comparison of 2 different points (first element). Result should be false.", mitk::Equal(p1,p2), false);
    itk::Point<mitk::ScalarType, 2> p3;
    p3[0] = 2.1;
    p3[1] = 2.1 + 1.01*mitk::eps;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Comparison of 2 different points (second element). Result should be false.", mitk::Equal(p1,p3), false);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkEqual)

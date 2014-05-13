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
#include "mitkTestFixture.h"

#include "mitkNumericConstants.h"
#include "mitkArray.h"


class mitkArrayTypeConversionTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkArrayTypeConversionTestSuite);

  MITK_TEST(EqualArray_ReturnsTrue);
  MITK_TEST(EqualArray_ReturnsFalse);

  MITK_TEST(FillVector3D_CorrectlyFilled);
  MITK_TEST(FillVector4D_CorrectlyFilled);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::ScalarType a[3];
  mitk::ScalarType b[3];

public:

  void setUp(void)
  {
    b[0] = a[0] = 1.0;
    b[1] = a[1] = 2.12;
    b[2] = a[2] = 3.13;
  }

  void tearDown(void)
  {

  }

  void EqualArray_ReturnsTrue(void)
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("test if EqualArray method returns true for two equal arrays.",
        true, mitk::EqualArray(a, b, 3));
  }

  void EqualArray_ReturnsFalse(void)
  {
    b[2] += mitk::eps;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("test if EqualArray method returns false for two non-equal arrays.",
        false, mitk::EqualArray(a, b, 3));
  }

  void FillVector3D_CorrectlyFilled(void)
  {
    mitk::ScalarType c[3];

    mitk::FillVector3D(c, a[0], a[1], a[2]);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("test if FillVector3D correctly fills array types.",
        true, mitk::EqualArray(a, c, 3));
  }

  void FillVector4D_CorrectlyFilled(void)
  {
    mitk::ScalarType e[4];
    mitk::ScalarType f[4];

    e[0] = 1.0;
    e[1] = 2.12;
    e[2] = 3.13;
    e[3] = 4.14;

    mitk::FillVector4D(f, e[0], e[1], e[2], e[3]);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("test if FillVector4D correctly fills array types.",
        true, mitk::EqualArray(e, f, 4));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkArrayTypeConversion)

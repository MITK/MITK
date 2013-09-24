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
#include "mitkPoint.h"
#include "mitkTypeOperations.h"
#include "mitkTypes.h" // for Equal method. TODO SW: Replace this once the Equal methods have their own header.

#include <iostream>

using namespace mitk;

static ScalarType bValues[3] = {14.123456, 10.2, 5.123456789};
static ScalarType cValues[3] = { 2.654321,  5.1, 2.543231111};

static Point3D a, b, c;


static void Setup(void)
{
  b = bValues;
  c = cValues;
}


template <typename TCoordRep, unsigned int NPointDimension>
static void TestForValuesHelper(itk::FixedArray<TCoordRep, NPointDimension>& result,
    ScalarType v0, ScalarType v1, ScalarType v2,
    std::string message, ScalarType eps = mitk::eps)
{
  MITK_TEST_CONDITION_REQUIRED(
         Equal(result[0], v0, eps)
      && Equal(result[1], v1, eps)
      && Equal(result[2], v2, eps), message)
}


static void Test_Addition(void)
{
  Setup();

  mitk::add(a, b, c);

  TestForValuesHelper(a, 16.777777, 15.3, 7.6666879, "summation a = b + c correctly performed");
}


static void Test_Substraction(void)
{
  Setup();

  mitk::sub(a, b, c);

  TestForValuesHelper(a, 11.469135, 5.1, 2.580225678, "difference a = b - c correctly performed");
}


static void Test_Multiplication(void)
{
  Setup();

  mitk::mul(a, b, c);

  TestForValuesHelper(a, 37.4881858534, 52.02, 13.0301347016, "multiplication a = b * c correctly performed", 1E-10);
  // google calculator provides us with only 10 digits after comma :)
}


static void Test_Division(void)
{
  Setup();

  mitk::div(a, b, c);

  TestForValuesHelper(a, 5.32092991014, 2.0, 2.01454628596, "division a = b / c correctly performed", 1E-11);
  // 11 digits after comma provided by google calculator :)
}


static void Test_DivisionByZero(void)
{
  Setup();
  c[2] = mitk::eps;

  MITK_TEST_FOR_EXCEPTION(mitk::Exception&, mitk::div(a,b,c, 2.0 * c[2]))
}


static void Test_DivisionByAlmostZero(void)
{
  Setup();
  c[2] = 2.0 * mitk::eps;

  mitk::div(a,b,c);

  MITK_TEST_CONDITION_REQUIRED(true, "division by element almost equal to zero doesn't throw exception")
}


int mitkTypeOperationTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("TypeOperationTest")

  Test_Addition();

  Test_Substraction();

  Test_Multiplication();

  Test_Division();
  Test_DivisionByZero();
  Test_DivisionByAlmostZero();

  MITK_TEST_END()

}

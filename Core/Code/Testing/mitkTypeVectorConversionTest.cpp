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

#include <iostream>

#include "itkVector.h"

#include <vnl/vnl_vector_ref.h>
#include <vnl/vnl_vector_fixed_ref.h>
#include "vnl/vnl_math.h"

#include "mitkTestingMacros.h"
#include "mitkTypeBasics.h"
#include "mitkVector.h"
#include "mitkTypes.h" // for Equals

using namespace mitk;

/**
 * these static variables are used in the test functions
 *
 * The variable which should be copied into is set to its original value.
 * The value which should be copied is set to valuesToCopy.
 *
 * Then the copying takes place. The test is successful, if the variable which
 * should be copied into holds the valuesToCopy afterwards and is equal to the
 * vector which should be copied.
 */
static const ScalarType originalValues[] =  {1.123456789987, 2.789456321456, 3.123654789987456};
static const ScalarType valuesToCopy[]   =  {4.654789123321, 5.987456789321, 6.321654987789546};

/**
 * @brief Convenience method to test if one vector has been assigned successfully to the other.
 *
 * More specifically, tests if v1 = v2 was performed correctly.
 *
 * @param v1    The vector v1 of the assignment v1 = v2
 * @param v2    The vector v2 of the assignment v1 = v2
 * @param v1Name        The type name of v1 (e.g.: mitk::Vector3D). Necessary for the correct test output.
 * @param v2Name        The type name of v2 (e.g.: mitk::Vector3D). Necessary for the correct test output.
*  @param eps   defines the allowed tolerance when testing for equality.
 */
template <typename T1, typename T2>
static void TestForEquality(T1 v1, T2 v2, std::string v1Name, std::string v2Name, ScalarType eps = mitk::eps)
{
  MITK_TEST_CONDITION( EqualArray(v1, v2, 3, eps),  "\nAssigning " << v2Name << " to " << v1Name << ":\n both are equal")
}

const float epsDouble2Float = vnl_math::float_eps * 10.0;



static void Test_pod2mitk(void)
{
  mitk::Vector3D vector3D = valuesToCopy;

  TestForEquality(vector3D, valuesToCopy, "mitk::Vector3D", "double POD");
}


static void Test_mitk2pod(void)
{
  ScalarType podArray[3];
  mitk::Vector3D vector3D = valuesToCopy;

  vector3D.ToArray(podArray);

  TestForEquality(podArray, vector3D, "double POD", "mitk::Vector3D");
}


static void Test_oneElement2mitk(void)
{
  double twos[] = {2.0, 2.0, 2.0};
  mitk::Vector<double, 3> vector3D(2.0);

  MITK_TEST_CONDITION(EqualArray(vector3D, twos, 3), "\none values initializes all elements to this value")
}


static void Test_itk2mitk(void)
{
  Vector3D vector3D = originalValues;
  itk::Vector<ScalarType, 3> itkVector = valuesToCopy;

  vector3D = itkVector;

  TestForEquality(vector3D, itkVector, "mitk::Vector3D", "itk::Vector");
}


static void Test_mitk2itk(void)
{
  Vector3D vector3D = valuesToCopy;
  itk::Vector<ScalarType, 3> itkVector = originalValues;

  itkVector = vector3D;

  TestForEquality(itkVector, vector3D, "itk::Vector", "mitk::Vector3D");
}


static void Test_vnlfixed2mitk(void)
{
  mitk::Vector3D vector3D = originalValues;
  vnl_vector_fixed<ScalarType, 3> vnlVectorFixed(valuesToCopy);

  vector3D = vnlVectorFixed;

  TestForEquality(vector3D, vnlVectorFixed, "mitk::Vector3D", "vnl_vector_fixed<ScalarType>");
}


static void Test_mitk2vnlfixed(void)
{
  vnl_vector_fixed<ScalarType, 3> vnlVectorFixed(originalValues);
  mitk::Vector3D vector3D = valuesToCopy;

  vnlVectorFixed = vector3D;

  TestForEquality(vnlVectorFixed, vector3D, "vnl_vector_fixed<ScalarType>", "mitk::Vector3D");
}


static void Test_vnl2mitk(void)
{
  mitk::Vector3D vector3D = originalValues;
  vnl_vector<ScalarType> vnlVector(3);
  vnlVector.set(valuesToCopy);

  vector3D = vnlVector;

  TestForEquality(vector3D, vnlVector, "mitk::Vector3D", "vnl_vector<ScalarType>");
}


static void Test_mitk2vnl(void)
{
  vnl_vector<ScalarType> vnlVector(3);
  vnlVector.set(originalValues);
  mitk::Vector3D vector3D = valuesToCopy;

  vnlVector = vector3D;

  TestForEquality(vnlVector, vector3D, "vnl_vector<ScalarType>", "mitk::Vector3D");
}


/**
 * @brief Tests if an exception is thrown when constructing an mitk::Vector form a vnl_vector of not suited size.
 */
static void Test_vnl2mitk_WrongVnlVectorSize()
{
  ScalarType largerValuesToCopy[] = {4.12345678910, 5.10987654321, 6.123456789132456, 7.123456987789456};
  mitk::Vector3D vector3D = originalValues;
  vnl_vector<ScalarType> vnlVector(4);
  vnlVector.set(largerValuesToCopy);

  MITK_TEST_FOR_EXCEPTION(mitk::Exception&, vector3D = vnlVector;)
}



static void Test_ToArray_DifferentType(void)
{
  float podArray[3];
  for (int var = 0; var < 3; ++var) {
    podArray[var] = originalValues[var];
  }
  mitk::Vector3D vector3D = valuesToCopy;

  vector3D.ToArray(podArray);

  TestForEquality(podArray, vector3D, "float POD", "mitk::Vector3D", epsDouble2Float);
}


static void Test_FromArray_DifferentType(void)
{
  mitk::Vector3D vector3D = originalValues;
  float podArray[3];
  for (int var = 0; var < 3; ++var) {
    podArray[var] = valuesToCopy[var];
  }

  vector3D.FromArray(podArray);

  TestForEquality(vector3D, podArray, "mitk::Vector3D", "float POD", epsDouble2Float);
}



/**
* @brief Test the conversions from and to the mitk::Vector type.
*
* Tests for every conversion, if it can be done and in a second test, if the assignment of a
* different type succeeds. E.g., assign a double vnl_vector to a float mitk::Vector
* In cases where the size can not be determined during compile time it is checked if the assignment of
* a differently sized vector yields an error.
*/
int mitkTypeVectorConversionTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("VectorConversionTest")

  Test_pod2mitk();
  Test_mitk2pod();

  Test_oneElement2mitk();

  Test_itk2mitk();
  Test_mitk2itk();

  Test_vnlfixed2mitk();
  Test_mitk2vnlfixed();

  Test_vnl2mitk();
  Test_mitk2vnl();
  Test_vnl2mitk_WrongVnlVectorSize();

  /**
   * The ToArray and FromArray can assign non equal types by implicit primitive type conversion.
   * The next two tests show this behavior
   */
  Test_ToArray_DifferentType();
  Test_FromArray_DifferentType();

  MITK_TEST_END()
}

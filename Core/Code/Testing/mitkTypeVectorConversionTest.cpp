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
 * these private variables are used in the test functions
 *
 * The variable which should be copied into is set to its original value.
 * The value which should be copied is set to valuesToCopy.
 *
 * Then the copying takes place. The test is successful, if the variable which
 * should be copied into holds the valuesToCopy afterwards and is equal to the
 * vector which should be copied.
 */
static const ScalarType originalValues[]      =  {1.123456789987, 2.789456321456, 3.123654789987456};
static const float      originalValuesFloat[] =  {1.123456789987, 2.789456321456, 3.123654789987456};

static const ScalarType valuesToCopy[]        =  {4.654789123321, 5.987456789321, 6.321654987789546};
static const float      valuesToCopyFloat[]   =  {4.654789123321, 5.987456789321, 6.321654987789546};

static float epsDouble2Float = vnl_math::float_eps * 10.0;

static void Test_pod2mitk(void)
{
  mitk::Vector3D vector3D = valuesToCopy;

  MITK_TEST_CONDITION(EqualArray(vector3D, valuesToCopy, 3), "pod copied into mitk::Vector of same type")
}

static void Test_pod2mitk_DifferentType(void)
{
  mitk::Vector3D vector3D = valuesToCopyFloat;

  MITK_TEST_CONDITION(EqualArray(vector3D, valuesToCopyFloat, 3, epsDouble2Float), "float pod copied into mitk::Vector<double>")
}


static void Test_mitk2pod(void)
{
  ScalarType podArray[3];
  mitk::Vector3D vector3D = valuesToCopy;

  vector3D.CopyToArray(podArray);

  MITK_TEST_CONDITION(EqualArray(podArray, vector3D, 3), "mitk::Vector copied into pod array")
}

static void Test_mitk2pod_DifferentType(void)
{
  float podArray[3];
  mitk::Vector3D vector3D = valuesToCopy;

  vector3D.CopyToArray(podArray);

  MITK_TEST_CONDITION(EqualArray(podArray, vector3D, 3, epsDouble2Float), "mitk::Vector<double> copied into float pod array")
}

static void Test_oneElement2mitk(void)
{
  double twos[] = {2.0, 2.0, 2.0};
  mitk::Vector<double, 3> vector3D(2.0);

  MITK_TEST_CONDITION(EqualArray(vector3D, twos, 3), "one values initializes all elements to this value")
}

static void Test_oneElement2mitk_DifferentType(void)
{
  float twos[] = {2.0, 2.0, 2.0};
  mitk::Vector<double, 3> vector3D(2.0F);

  MITK_TEST_CONDITION(EqualArray(vector3D, twos, 3), "one values initializes all elements to this value: value has different type than mitk::Vector.")
}

static void Test_mitk2mitk_DifferentType(void)
{
  mitk::Vector<float, 3>  floatVector3D  = originalValuesFloat;
  mitk::Vector<double, 3> doubleVector3D = valuesToCopy;

  floatVector3D = doubleVector3D;

  MITK_TEST_CONDITION(floatVector3D == doubleVector3D, "mitk::Vector<double> assigned to mitk::Vector<float>")
  MITK_TEST_CONDITION(EqualArray(floatVector3D, valuesToCopy, 3, epsDouble2Float), "correct values were assigned within float accuracy")
}

static void Test_itk2mitk(void)
{
  Vector3D vector3D = originalValues;
  itk::Vector<ScalarType, 3> itkVector = valuesToCopy;

  vector3D = itkVector;

  MITK_TEST_CONDITION(vector3D == itkVector, "itk::Vector assigned to mitk::Vector")
  MITK_TEST_CONDITION(vector3D == valuesToCopy, "correct values were assigned")
}

static void Test_itk2mitk_DifferentType(void)
{
  Vector<float, 3> vector3F        = originalValuesFloat;
  itk::Vector<double, 3> itkVector = valuesToCopy;

  vector3F = itkVector;

  MITK_TEST_CONDITION(EqualArray(vector3F, itkVector, 3, epsDouble2Float) , "itk::Vector<double> assigned to mitk::Vector<float>")
  MITK_TEST_CONDITION(EqualArray(vector3F, valuesToCopyFloat, 3, epsDouble2Float), "correct values were assigned")
}

static void Test_mitk2itk(void)
{
  Vector3D vector3D = valuesToCopy;
  itk::Vector<ScalarType, 3> itkVector = originalValues;

  itkVector = vector3D;

  MITK_TEST_CONDITION(vector3D == itkVector, "mitk::Vector assigned to itk::Vector")
  MITK_TEST_CONDITION(itkVector == valuesToCopy, "correct values were assigned")
}

static void Test_mitk2itk_DifferentType(void)
{
  itk::Vector<float,3> itkFloatVector = originalValuesFloat;
  mitk::Vector<double, 3> mitkDoubleVector = valuesToCopy;

  itkFloatVector = mitkDoubleVector;

  MITK_TEST_CONDITION(itkFloatVector == mitkDoubleVector, "mitk::Vector<double> assigned to itk::Vector<float>")
  MITK_TEST_CONDITION(EqualArray(itkFloatVector, valuesToCopy, 3, epsDouble2Float), "correct values were assigned")
}


static void Test_vnlfixed2mitk(void)
{
  mitk::Vector3D vector3D = originalValues;
  vnl_vector_fixed<ScalarType, 3> vnlVectorFixed(valuesToCopy);

  vector3D = vnlVectorFixed;

  MITK_TEST_CONDITION( vector3D.GetVnlVector() == vnlVectorFixed, "vnl_vector_fixed assigned to mitk::Vector")
  MITK_TEST_CONDITION( vector3D == valuesToCopy, "correct values were assigned" )
}

static void Test_vnlfixed2mitk_DifferentType(void)
{
  mitk::Vector<float, 3> vector3F = originalValuesFloat;
  vnl_vector_fixed<double, 3> vnlVectorFixed(valuesToCopy);

  vector3F = vnlVectorFixed;

  MITK_TEST_CONDITION( EqualArray(vector3F, vnlVectorFixed, 3, epsDouble2Float), "vnl_vector_fixed<double> assigned to mitk::Vector<float>")
  MITK_TEST_CONDITION( EqualArray(vector3F, valuesToCopy,   3, epsDouble2Float), "correct values were assigned" )
}

static void Test_mitk2vnlfixed(void)
{
  vnl_vector_fixed<ScalarType, 3> vnlVectorFixed(originalValues);
  mitk::Vector3D vector3D = valuesToCopy;

  vnlVectorFixed = vector3D;

  MITK_TEST_CONDITION( vnlVectorFixed == vector3D.GetVnlVector(),   "mitk::Vector assigned to vnl_vector_fixed")
  MITK_TEST_CONDITION( EqualArray(vnlVectorFixed, valuesToCopy, 3), "correct values were assigned" )
}

static void Test_mitk2vnlfixed_DifferentType(void)
{
  vnl_vector_fixed<float, 3> vnlVectorFixed(originalValuesFloat);
  mitk::Vector<double, 3> vector3D = valuesToCopy;

  vnlVectorFixed = vector3D;

  MITK_TEST_CONDITION( EqualArray(vnlVectorFixed, vector3D,     3, epsDouble2Float), "mitk::Vector<double> assigned to vnl_vector_fixed<float>")
  MITK_TEST_CONDITION( EqualArray(vnlVectorFixed, valuesToCopy, 3, epsDouble2Float), "correct values were assigned" )
}


static void Test_vnl2mitk(void)
{

  mitk::Vector3D vector3D = originalValues;
  vnl_vector<ScalarType> vnlVector(3);
  vnlVector.set(valuesToCopy);

  vector3D = vnlVector;

  MITK_TEST_CONDITION( vector3D.GetVnlVector() == vnlVector, "vnl_vector assigned to mitk::Vector")
  MITK_TEST_CONDITION( vector3D == valuesToCopy,             "correct values were assigned" )
}

static void Test_vnl2mitk_DifferentType(void)
{

  mitk::Vector<float, 3> vector3F = originalValuesFloat;
  vnl_vector<double> vnlVector(3);
  vnlVector.set(valuesToCopy);

  vector3F = vnlVector;

  MITK_TEST_CONDITION( EqualArray(vector3F, vnlVector,    3, epsDouble2Float), "vnl_vector<double> assigned to mitk::Vector<float>")
  MITK_TEST_CONDITION( EqualArray(vector3F, valuesToCopy, 3, epsDouble2Float), "correct values were assigned" )
}

static void Test_mitk2vnl(void)
{
  vnl_vector<ScalarType> vnlVector(3);
  vnlVector.set(originalValues);
  mitk::Vector3D vector3D = valuesToCopy;

  vnlVector = vector3D;

  MITK_TEST_CONDITION( vnlVector == vector3D.GetVnlVector(),   "mitk::Vector assigned to vnl_vector")
  MITK_TEST_CONDITION( EqualArray(vnlVector, valuesToCopy, 3), "correct values were assigned" )
}

static void Test_mitk2vnl_DifferentType(void)
{
  vnl_vector<float> vnlVector(3);
  vnlVector.set(originalValuesFloat);
  mitk::Vector3D vector3D = valuesToCopy;

  vnlVector = vector3D;

  MITK_TEST_CONDITION( EqualArray(vnlVector, vector3D,     3, epsDouble2Float), "mitk::Vector<double> assigned to vnl_vector<float>")
  MITK_TEST_CONDITION( EqualArray(vnlVector, valuesToCopy, 3, epsDouble2Float), "correct values were assigned" )
}


/**
 * tests if constructing a mitk::Vector using a too large in size vnl_vector is handled properly.
 * Meaning: Exception is thrown.
 */
static void Test_vnl2mitk_WrongVnlVectorSize()
{
  ScalarType largerValuesToCopy[] = {4.12345678910, 5.10987654321, 6.123456789132456, 7.123456987789456};
  mitk::Vector3D vector3D = originalValues;
  vnl_vector<ScalarType> vnlVector(4);
  vnlVector.set(largerValuesToCopy);

  MITK_TEST_FOR_EXCEPTION(mitk::Exception&, vector3D = vnlVector;)

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

  Test_mitk2mitk_DifferentType();

  Test_pod2mitk();
  Test_pod2mitk_DifferentType();

  Test_mitk2pod();
  Test_mitk2pod_DifferentType();

  Test_oneElement2mitk();
  Test_oneElement2mitk_DifferentType();

  Test_itk2mitk();
  Test_itk2mitk_DifferentType();

  Test_mitk2itk();
  Test_mitk2itk_DifferentType();

  Test_vnlfixed2mitk();
  Test_vnlfixed2mitk_DifferentType();

  Test_mitk2vnlfixed();
  Test_mitk2vnlfixed_DifferentType();

  Test_vnl2mitk();
  Test_vnl2mitk_DifferentType();
  Test_vnl2mitk_WrongVnlVectorSize();

  Test_mitk2vnl();
  Test_mitk2vnl_DifferentType();

  MITK_TEST_END()

}

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


static void Test_pod2mitk(void)
{
  mitk::Vector3D vector3D = valuesToCopy;

  MITK_TEST_CONDITION(EqualArray(vector3D, valuesToCopy, 3), "pod copied into vector of same type")
}

static void Test_pod2mitk_DifferentType(void)
{
  mitk::Vector3D vector3D = valuesToCopyFloat;

  MITK_TEST_CONDITION(EqualArray(vector3D, valuesToCopyFloat, 3, vnl_math::float_eps), "float pod copied into double vector")
}


static void Test_mitk2pod(void)
{
  ScalarType podArray[3];
  mitk::Vector3D vector3D = valuesToCopy;

  vector3D.CopyToArray(podArray);

  MITK_TEST_CONDITION(EqualArray(podArray, vector3D, 3), "mitk vector copied into pod array")
}

static void Test_mitk2pod_DifferentType(void)
{
  float podArray[3];
  mitk::Vector3D vector3D = valuesToCopy;

  vector3D.CopyToArray(podArray);

  MITK_TEST_CONDITION(EqualArray(podArray, vector3D, 3, vnl_math::float_eps*10.0), "mitk vector copied into pod array with different type")
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

  MITK_TEST_CONDITION(EqualArray(vector3D, twos, 3), "one values initializes all elements to this value: value has different type than vector.")
}

static void Test_mitk2mitk_DifferentType(void)
{
  mitk::Vector<float, 3>  floatVector3D  = originalValuesFloat;
  mitk::Vector<double, 3> doubleVector3D = valuesToCopy;

  floatVector3D = doubleVector3D;

  MITK_TEST_CONDITION(floatVector3D == doubleVector3D, "mitk double vector assigned to mitk float vector")
  MITK_TEST_CONDITION(EqualArray(floatVector3D, valuesToCopy, 3, vnl_math::float_eps * 10.0), "correct values were assigned within float accuracy")
}

static void Test_mitk2mitk_ShorterVectorAssigned(void)
{
  mitk::Vector3D vector3D = originalValues;
  mitk::Vector2D vector2D = valuesToCopy;

  vector3D = vector2D;

  bool firstTwoElementsCopied = EqualArray(vector3D, vector2D, 2);
  bool lastElementIsUntouched = (vector3D[2] == originalValues[2]);

  MITK_TEST_CONDITION(firstTwoElementsCopied, "shorter mitk vector correctly assigned to mitk vector: first two elements were copied" )
  MITK_TEST_CONDITION(lastElementIsUntouched, "shorter mitk vector correctly assigned to mitk vector: last element was untoched")

}

static void Test_itk2mitk(void)
{
  Vector3D vector3D = originalValues;
  itk::Vector<ScalarType, 3> itkVector = valuesToCopy;

  vector3D = itkVector;

  MITK_TEST_CONDITION(vector3D == itkVector, "itk vector assigned to mitk vector")
  MITK_TEST_CONDITION(vector3D == valuesToCopy, "correct values were assigned")
}

static void Test_itk2mitk_DifferentType(void)
{
  Vector3D vector3D = originalValues;
  itk::Vector<float, 3> itkVector = valuesToCopyFloat;

  vector3D = itkVector;

  MITK_TEST_CONDITION(vector3D == itkVector, "itk float vector assigned to mitk double vector")
  MITK_TEST_CONDITION(EqualArray(vector3D, valuesToCopyFloat, 3), "correct values were assigned")
}

static void Test_mitk2itk(void)
{
  Vector3D vector3D = valuesToCopy;
  itk::Vector<ScalarType, 3> itkVector = originalValues;

  itkVector = vector3D;

  MITK_TEST_CONDITION(vector3D == itkVector, "mitk vector assigned to itk vector")
  MITK_TEST_CONDITION(itkVector == valuesToCopy, "correct values were assigned")
}

static void Test_mitk2itk_DifferentType(void)
{
  itk::Vector<double,3> itkDoubleVector = originalValues;
  mitk::Vector<float, 3> mitkFloatVector = valuesToCopyFloat;

  itkDoubleVector = mitkFloatVector;

  MITK_TEST_CONDITION(itkDoubleVector == mitkFloatVector, "mitk float vector assigned to itk double vector")
  MITK_TEST_CONDITION(EqualArray(itkDoubleVector, valuesToCopyFloat, 3), "correct values were assigned")
}


static void Test_vnlfixed2mitk(void)
{
  mitk::Vector3D vector3D = originalValues;
  vnl_vector_fixed<ScalarType, 3> vnlVectorFixed(valuesToCopy);

  vector3D = vnlVectorFixed;

  MITK_TEST_CONDITION( vector3D.GetVnlVector() == vnlVectorFixed, "vnl_vector_fixed assigned to mitk vector")
  MITK_TEST_CONDITION( vector3D == valuesToCopy, "correct values were assigned" )
}

static void Test_vnlfixed2mitk_DifferentType(void)
{
  mitk::Vector<double, 3> vector3D = originalValues;
  vnl_vector_fixed<float, 3> vnlVectorFixed(valuesToCopyFloat);

  vector3D = vnlVectorFixed;

  MITK_TEST_CONDITION( EqualArray(vector3D, vnlVectorFixed, 3), "vnl_vector_fixed float assigned to mitk vector double")
  MITK_TEST_CONDITION( EqualArray(vector3D, valuesToCopyFloat, 3),   "correct values were assigned" )
}

static void Test_mitk2vnlfixed(void)
{
  vnl_vector_fixed<ScalarType, 3> vnlVectorFixed(originalValues);
  mitk::Vector3D vector3D = valuesToCopy;

  vnlVectorFixed = vector3D;

  MITK_TEST_CONDITION( vnlVectorFixed == vector3D.GetVnlVector(), "mitk vector assigned to vnl_vector_fixed")
  MITK_TEST_CONDITION( EqualArray(vnlVectorFixed, valuesToCopy, 3), "correct values were assigned" )
}

static void Test_mitk2vnlfixed_DifferentType(void)
{
  vnl_vector_fixed<float, 3> vnlVectorFixed(originalValuesFloat);
  mitk::Vector<double, 3> vector3D = valuesToCopy;

  vnlVectorFixed = vector3D;

  MITK_TEST_CONDITION( EqualArray(vnlVectorFixed, vector3D, 3, vnl_math::float_eps * 10.0), "mitk vector double assigned to vnl_vector_fixed float")
  MITK_TEST_CONDITION( EqualArray(vnlVectorFixed, valuesToCopy, 3, vnl_math::float_eps * 10.0), "correct values were assigned" )
}


static void Test_vnl2mitk(void)
{

  mitk::Vector3D vector3D = originalValues;
  vnl_vector<ScalarType> vnlVector(3);
  vnlVector.set(valuesToCopy);

  vector3D = vnlVector;

  MITK_TEST_CONDITION( vector3D.GetVnlVector() == vnlVector, "vnl_vector assigned to mitk vector")
  MITK_TEST_CONDITION( vector3D == valuesToCopy, "correct values were assigned" )
}

static void Test_vnl2mitk_DifferentType(void)
{

  mitk::Vector<double, 3> vector3D = originalValues;
  vnl_vector<float> vnlVector(3);
  vnlVector.set(valuesToCopyFloat);

  vector3D = vnlVector;

  MITK_TEST_CONDITION( EqualArray(vector3D, vnlVector, 3, vnl_math::float_eps * 10.0), "vnl_vector float assigned to mitk vector double")
  MITK_TEST_CONDITION( EqualArray(vector3D, valuesToCopy, 3,  vnl_math::float_eps * 10.0), "correct values were assigned" )
}

/**
 * tests if constructing a mitk::Vector using a shorter in size vnl_vector is handled properly.
 * Attention:
 * say you have a mitk::Vector holding {1, 2, 3}.
 * Now you assign a vnl_vector holdin {4, 5}.
 *
 * The result will be {4, 5, 0}, not {4, 5, 3}
 */
static void Test_vnl2mitk_ShorterVectorInConstructor(void)
{
  vnl_vector<ScalarType> vnlVector(2);
  vnlVector.set(valuesToCopy);

  mitk::Vector3D vector3D = vnlVector;

  bool firstTwoElementsCopied = EqualArray(vector3D, vnlVector, 2);
  bool lastElementIs0 = (vector3D[2] == 0.0);

  MITK_TEST_CONDITION(
      firstTwoElementsCopied && lastElementIs0, "shorter vnl vector correctly copied to mitk vector" )
}

//static void Test_vnl2mitk_ShorterVectorAssigned()
//{
//  mitk::Vector3D vector3D = originalValues;
//  vnl_vector<ScalarType> vnlVector(2);
//  vnlVector.set(valuesToCopy);
//
//  vector3D = vnlVector;
//
//  bool firstTwoElementsCopied = EqualArray(vector3D, vnlVector, 2);
//  bool lastElementUnchanged = (vector3D[2] == originalValues[2]);
//
//  MITK_TEST_CONDITION(
//      firstTwoElementsCopied && lastElementUnchanged, "shorter vnl vector correctly assigned to mitk vector" )
//}

static void Test_mitk2vnl(void)
{
  vnl_vector<ScalarType> vnlVector(3);
  vnlVector.set(originalValues);
  mitk::Vector3D vector3D = valuesToCopy;

  vnlVector = vector3D;

  MITK_TEST_CONDITION( vnlVector == vector3D.GetVnlVector(), "mitk vector assigned to vnlVector")
  MITK_TEST_CONDITION( EqualArray(vnlVector, valuesToCopy, 3), "correct values were assigned" )
}

static void Test_mitk2vnl_DifferentType(void)
{
  vnl_vector<float> vnlVector(3);
  vnlVector.set(originalValuesFloat);
  mitk::Vector3D vector3D = valuesToCopy;

  vnlVector = vector3D;

  MITK_TEST_CONDITION( EqualArray(vnlVector, vector3D, 3, vnl_math::float_eps * 10.0), "mitk vector double assigned to vnlVector float")
  MITK_TEST_CONDITION( EqualArray(vnlVector, valuesToCopy, 3, vnl_math::float_eps * 10.0), "correct values were assigned" )
}




/**
 * tests if constructing a mitk::Vector using a large in size vnl_vector is handled properly
 */
static void Test_vnl2mitk_LargerVectorConstructor()
{
  ScalarType largerValuesToCopy[] = {4.12345678910, 5.10987654321, 6.123456789132456, 7.123456987789456};
  mitk::Vector3D vector3D = originalValues;
  vnl_vector<ScalarType> vnlVector(4);
  vnlVector.set(largerValuesToCopy);

  vector3D = vnlVector;

  MITK_TEST_CONDITION(EqualArray(vector3D, largerValuesToCopy, 3), "larger vnl vector correctly assigned to mitk vector" )
}



/**
* Test the conversions from and to the mitk vector type
*/
int mitkTypeVectorConversionTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("VectorConversionTest")

  Test_pod2mitk();
  Test_pod2mitk_DifferentType();

  Test_mitk2pod();
  Test_mitk2pod_DifferentType();

  Test_oneElement2mitk();
  Test_oneElement2mitk_DifferentType();

  Test_mitk2mitk_DifferentType();
  Test_mitk2mitk_ShorterVectorAssigned();

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
  Test_vnl2mitk_ShorterVectorInConstructor();
  //Test_vnl2mitk_ShorterVectorAssigned();
  Test_vnl2mitk_LargerVectorConstructor();

  Test_mitk2vnl();
  Test_mitk2vnl_DifferentType();

  MITK_TEST_END()

}

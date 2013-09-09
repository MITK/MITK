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



static void Test_mitk2Mitk_DifferntTypeCompatibility(void)
{
  mitk::Vector<float, 3>  floatVector3D  = originalValuesFloat;
  mitk::Vector<double, 3> doubleVector3D = valuesToCopy;

  floatVector3D = doubleVector3D;

  MITK_TEST_CONDITION(floatVector3D == doubleVector3D, "mitk double vector assigned to mitk float vector")

  // test for correct values needs a little more love than in other cases
  // since Equal cannot compare arrays/vectors of different type
  bool correctValuesAssigned = true;
  for (int var = 0; var < 3; ++var) {
      correctValuesAssigned = correctValuesAssigned
          && Equal(floatVector3D[var], valuesToCopy[var], vnl_math::float_eps * 10.0);
  }
  MITK_TEST_CONDITION(correctValuesAssigned, "correct values were assigned within float accuracy")
}

static void Test_itk2Mitk_Compatibility(void)
{
  Vector3D vector3D = originalValues;
  itk::Vector<ScalarType, 3> itkVector = valuesToCopy;

  vector3D = itkVector;

  MITK_TEST_CONDITION(vector3D == itkVector, "itk vector assigned to mitk vector")
  MITK_TEST_CONDITION(vector3D == valuesToCopy, "correct values were assigned")
}

static void Test_itk2Mitk_floatCompatibility(void)
{
  Vector3D vector3D = originalValues;
  itk::Vector<float, 3> itkVector = valuesToCopyFloat;

  vector3D = itkVector;

  MITK_TEST_CONDITION(vector3D == itkVector, "itk float vector assigned to mitk vector")

  // test for correct values needs a little more love than in other cases
  // since Equal cannot compare arrays/vectors of different type
  bool correctValuesAssigned = true;
  for (int var = 0; var < 3; ++var) {
      correctValuesAssigned = correctValuesAssigned
          && Equal(vector3D[var], valuesToCopyFloat[var]);
  }
  MITK_TEST_CONDITION(correctValuesAssigned, "correct values were assigned")
}

static void Test_Mitk2itk_Compatibility(void)
{
  Vector3D vector3D = valuesToCopy;
  itk::Vector<ScalarType, 3> itkVector = originalValues;

  itkVector = vector3D;

  MITK_TEST_CONDITION(vector3D == itkVector, "mitk vector assigned to itk vector")
  MITK_TEST_CONDITION(itkVector == valuesToCopy, "correct values were assigned")
}



static void Test_Vnl2Mitk_VectorFixedCompatibility()
{

  mitk::Vector3D vector3D = originalValues;
  vnl_vector_fixed<ScalarType, 3> vnlVectorFixed(valuesToCopy);

  vector3D = vnlVectorFixed;

  MITK_TEST_CONDITION( vector3D.GetVnlVector() == vnlVectorFixed, "vnl_vector_fixed assigned to mitk vector")
  MITK_TEST_CONDITION( vector3D == valuesToCopy, "correct values were assigned" )
}

static void Test_Vnl2Mitk_VectorCompatibility()
{

  mitk::Vector3D vector3D = originalValues;
  vnl_vector<ScalarType> vnlVector(3);
  vnlVector.set(valuesToCopy);

  vector3D = vnlVector;

  MITK_TEST_CONDITION( vector3D.GetVnlVector() == vnlVector, "vnl_vector assigned to mitk vector")
  MITK_TEST_CONDITION( vector3D == valuesToCopy, "correct values were assigned" )
}

/**
 * tests if constructing a mitk::Vector using a shorter in size vnl_vector is handled properly
 */
static void Test_Vnl2Mitk_ShortVectorCompatibility()
{
  ScalarType shorterValuesToCopy[] = {4.12345678910, 5.10987654321};
  mitk::Vector3D vector3D = originalValues;
  vnl_vector<ScalarType> vnlVector(2);
  vnlVector.set(shorterValuesToCopy);

  vector3D = vnlVector;

  bool correctValuesInVector3D = true;
  for (int var = 0; var < 2; ++var) {
    correctValuesInVector3D = correctValuesInVector3D &&
        (vector3D[var] && shorterValuesToCopy[var]);
  }
  correctValuesInVector3D = correctValuesInVector3D && (vector3D[2] == 0.0);

  MITK_TEST_CONDITION(correctValuesInVector3D, "shorter vnl vector correctly assigned to mitk vector" )
}

/**
 * tests if constructing a mitk::Vector using a large in size vnl_vector is handled properly
 */
static void Test_Vnl2Mitk_LargeVectorCompatibility()
{
  ScalarType largerValuesToCopy[] = {4.12345678910, 5.10987654321, 6.123456789132456, 7.123456987789456};
  mitk::Vector3D vector3D = originalValues;
  vnl_vector<ScalarType> vnlVector(4);
  vnlVector.set(largerValuesToCopy);

  vector3D = vnlVector;

  bool correctValuesInVector3D = true;
  for (int var = 0; var < 3; ++var) {
    correctValuesInVector3D = correctValuesInVector3D &&
        (vector3D[var] && largerValuesToCopy[var]);
  }

  MITK_TEST_CONDITION(correctValuesInVector3D, "larger vnl vector correctly assigned to mitk vector" )
}


/**
* Test the conversions from and to the mitk vector type
*/
int mitkTypeVectorConversionTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("VectorConversionTest")

  Test_mitk2Mitk_DifferntTypeCompatibility();

  Test_itk2Mitk_Compatibility();
  Test_itk2Mitk_floatCompatibility();

  Test_Mitk2itk_Compatibility();

  Test_Vnl2Mitk_VectorFixedCompatibility();

  Test_Vnl2Mitk_VectorCompatibility();
  Test_Vnl2Mitk_ShortVectorCompatibility();
  Test_Vnl2Mitk_LargeVectorCompatibility();

  MITK_TEST_END()

}

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
#include "mitkTypeBasics.h"
#include "mitkVector.h"
#include "mitkTypes.h" // for Equals
#include "itkVector.h"
#include <vnl/vnl_vector_ref.h>
#include <vnl/vnl_vector_fixed_ref.h>

#include <iostream>

using namespace mitk;


static const ScalarType originalValues[] =  {1.0, 2.0, 3.0};
static const ScalarType valuesToCopy[]   =  {4.0, 5.0, 6.0};


static void Test_itk2Mitk_Compatibility(void)
{
  Vector3D vector3D = originalValues;
  itk::Vector<ScalarType, 3> itkVector = valuesToCopy;

  vector3D = itkVector;

  MITK_TEST_CONDITION(vector3D == itkVector, "itk vector assigned to mitk vector")
  MITK_TEST_CONDITION(vector3D == valuesToCopy, "correct values were assigned")
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
  MITK_TEST_CONDITION( Equal(vnlVectorFixed, vnlVectorFixed), "vnl_vector_fixed holds its original values" )
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

  Test_itk2Mitk_Compatibility();
  Test_Mitk2itk_Compatibility();

  Test_Vnl2Mitk_VectorFixedCompatibility();

  Test_Vnl2Mitk_VectorCompatibility();
  Test_Vnl2Mitk_ShortVectorCompatibility();
  Test_Vnl2Mitk_LargeVectorCompatibility();

  MITK_TEST_END()

}

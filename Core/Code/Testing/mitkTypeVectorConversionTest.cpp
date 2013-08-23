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


template< class T, unsigned int NVectorDimension>
itk::Vector<T, NVectorDimension>  toItk(const vnl_vector_fixed<T, NVectorDimension>& vnlVectorFixed)
{
  return toItk<T, NVectorDimension>(vnlVectorFixed.as_vector());
}


template< class T, unsigned int NVectorDimension>
itk::Vector<T, NVectorDimension>  toItk(const vnl_vector<T>& vnlVector)
{
  itk::Vector<T, NVectorDimension> vector;
  vector.SetVnlVector(vnlVector);
  return vector;
}


static void Test_Vnl2Mitk_VectorFixedCompatibility()
{

  mitk::Vector3D vector3D = originalValues;
  vnl_vector_fixed<ScalarType, 3> vnlVectorFixed(valuesToCopy);

  vector3D = toItk(vnlVectorFixed);

  MITK_TEST_CONDITION( vector3D.GetVnlVector() == vnlVectorFixed, "vnl_vector_fixed assigned to mitk vector")
  MITK_TEST_CONDITION( vector3D == valuesToCopy, "correct values were assigned" )
}

static void Test_Vnl2Mitk_VectorCompatibility()
{

  mitk::Vector3D vector3D = originalValues;
  vnl_vector<ScalarType> vnlVector(3);
  vnlVector.set(valuesToCopy);

  vector3D = toItk<ScalarType, 3>(vnlVector);

  MITK_TEST_CONDITION( vector3D.GetVnlVector() == vnlVector, "vnl_vector assigned to mitk vector")
  MITK_TEST_CONDITION( vector3D == valuesToCopy, "correct values were assigned" )
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

  MITK_TEST_END()

}

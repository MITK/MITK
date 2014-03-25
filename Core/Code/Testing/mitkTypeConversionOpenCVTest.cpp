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

#include <cv.h>

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




static void Test_mitk2opencv()
{
  cv::Vec3d opencvVector(originalValues[0], originalValues[1], originalValues[2]);
  mitk::Vector3D vector3D = valuesToCopy;

  opencvVector = vector3D.ToArray<cv::Vec3d>();

  TestForEquality(opencvVector, vector3D, "cv::Vec3d", "mitk::Vector3D");
}


static void Test_opencv2mitk()
{
  mitk::Vector3D vector3D = originalValues;
  cv::Vec3d opencvVector(valuesToCopy[0], valuesToCopy[1], valuesToCopy[2]);

  vector3D.FromArray(opencvVector);

  TestForEquality(vector3D, opencvVector, "mitk::Vector3D", "cv::Vec3d");
}


/**
* @brief Test the conversions from and to opencv from mitk types.
*
*/
int mitkTypeConversionOpenCVTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("OpenCVConversionTest")

  Test_mitk2opencv();
  Test_opencv2mitk();

  MITK_TEST_END()
}

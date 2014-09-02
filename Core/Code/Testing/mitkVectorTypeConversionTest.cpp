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

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <vnl/vnl_vector_fixed_ref.h>
#include "vnl/vnl_math.h"

#include "mitkNumericConstants.h"
#include "mitkVector.h"
#include "mitkPoint.h"

using namespace mitk;


class mitkVectorTypeConversionTestSuite : public mitk::TestFixture

{

  CPPUNIT_TEST_SUITE(mitkVectorTypeConversionTestSuite);

  MITK_TEST(Point2Vector);

  MITK_TEST(Pod2Mitk);
  MITK_TEST(Mitk2Pod);

  MITK_TEST(OneElement2Mitk);

  MITK_TEST(Itk2Mitk);
  MITK_TEST(Mitk2Itk);

  MITK_TEST(Vnlfixed2Mitk);
  MITK_TEST(Mitk2Vnlfixed);

  MITK_TEST(Vnl2Mitk);
  MITK_TEST(Mitk2Vnl);
  MITK_TEST(Vnl2Mitk_WrongVnlVectorSize);

  MITK_TEST(ToArray_DifferentType);
  MITK_TEST(Fill_DifferentType);

  CPPUNIT_TEST_SUITE_END();

private:

  /**
   * these variables are used in the test functions
   *
   * The variable which should be copied into is set to its original value.
   * The value which should be copied is set to valuesToCopy.
   *
   * Then the copying takes place. The test is successful, if the variable which
   * should be copied into holds the valuesToCopy afterwards and is equal to the
   * vector which should be copied.
   */
  ScalarType originalValues[3];
  ScalarType valuesToCopy[3];

  float epsDouble2Float;

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
  void TestForEquality(const T1& v1, const T2& v2, const std::string& v1Name, const std::string& v2Name, const ScalarType& eps = mitk::eps) const
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("\nAssigning " + v2Name + " to " + v1Name + ":\n both are equal", true,
        EqualArray(v1, v2, 3, eps));
  }

public:


  void setUp(void)
  {
    FillVector3D(originalValues, 1.123456789987, 2.789456321456, 3.123654789987456);
    FillVector3D(valuesToCopy,   4.654789123321, 5.987456789321, 6.321654987789546);

    epsDouble2Float = vnl_math::float_eps * 10.0;
  }

  void tearDown(void)
  {

  }


  void Pod2Mitk(void)
  {
    mitk::Vector3D vector3D = valuesToCopy;

    TestForEquality(vector3D, valuesToCopy, "mitk::Vector3D", "double POD");
  }


  void Mitk2Pod(void)
  {
    ScalarType podArray[3];
    mitk::Vector3D vector3D = valuesToCopy;

    vector3D.ToArray(podArray);

    TestForEquality(podArray, vector3D, "double POD", "mitk::Vector3D");
  }


  void OneElement2Mitk(void)
  {
    double twos[] = {2.0, 2.0, 2.0};
    mitk::Vector<double, 3> vector3D(2.0);

    CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "\n one values initializes all elements to this value", EqualArray(vector3D, twos, 3));
  }


  void Itk2Mitk(void)
  {
    Vector3D vector3D = originalValues;
    itk::Vector<ScalarType, 3> itkVector = valuesToCopy;

    vector3D = itkVector;

    TestForEquality(vector3D, itkVector, "mitk::Vector3D", "itk::Vector");
  }


  void Mitk2Itk(void)
  {
    Vector3D vector3D = valuesToCopy;
    itk::Vector<ScalarType, 3> itkVector = originalValues;

    itkVector = vector3D;

    TestForEquality(itkVector, vector3D, "itk::Vector", "mitk::Vector3D");
  }


  void Vnlfixed2Mitk(void)
  {
    mitk::Vector3D vector3D = originalValues;
    vnl_vector_fixed<ScalarType, 3> vnlVectorFixed(valuesToCopy);

    vector3D = vnlVectorFixed;

    TestForEquality(vector3D, vnlVectorFixed, "mitk::Vector3D", "vnl_vector_fixed<ScalarType>");
  }


  void Mitk2Vnlfixed(void)
  {
    vnl_vector_fixed<ScalarType, 3> vnlVectorFixed(originalValues);
    mitk::Vector3D vector3D = valuesToCopy;

    vnlVectorFixed = vector3D;

    TestForEquality(vnlVectorFixed, vector3D, "vnl_vector_fixed<ScalarType>", "mitk::Vector3D");
  }


  void Vnl2Mitk(void)
  {
    mitk::Vector3D vector3D = originalValues;
    vnl_vector<ScalarType> vnlVector(3);
    vnlVector.set(valuesToCopy);

    vector3D = vnlVector;

    TestForEquality(vector3D, vnlVector, "mitk::Vector3D", "vnl_vector<ScalarType>");
  }


  void Mitk2Vnl(void)
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
  void Vnl2Mitk_WrongVnlVectorSize()
  {
    ScalarType largerValuesToCopy[] = {4.12345678910, 5.10987654321, 6.123456789132456, 7.123456987789456};
    mitk::Vector3D vector3D = originalValues;
    vnl_vector<ScalarType> vnlVector(4);
    vnlVector.set(largerValuesToCopy);

    CPPUNIT_ASSERT_THROW(vector3D = vnlVector, mitk::Exception);
  }


  void ToArray_DifferentType(void)
  {
    float podArray[3];
    for (int var = 0; var < 3; ++var) {
      podArray[var] = originalValues[var];
    }
    mitk::Vector3D vector3D = valuesToCopy;

    vector3D.ToArray(podArray);

    TestForEquality(podArray, vector3D, "float POD", "mitk::Vector3D", epsDouble2Float);
  }


  void Fill_DifferentType(void)
  {
    mitk::Vector3D vector3D = originalValues;
    float podArray[3];
    for (int var = 0; var < 3; ++var) {
      podArray[var] = valuesToCopy[var];
    }

    vector3D.FillVector(podArray);

    TestForEquality(vector3D, podArray, "mitk::Vector3D", "float POD", epsDouble2Float);
  }

  void Point2Vector()
  {
    mitk::Point3D point3D   = originalValues;
    mitk::Vector3D vector3D = valuesToCopy;

    vector3D = point3D.GetVectorFromOrigin();

    TestForEquality(point3D, vector3D, "mitk::Point3D", "mitk::Vector3D");
  }


};

MITK_TEST_SUITE_REGISTRATION(mitkVectorTypeConversion)

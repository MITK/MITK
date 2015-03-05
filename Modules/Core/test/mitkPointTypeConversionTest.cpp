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

#include "mitkTestFixture.h"

#include "mitkTestingMacros.h"
#include "mitkNumericConstants.h"
#include "mitkNumericTypes.h" // for Equal method
#include "mitkPoint.h"
#include "itkPoint.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"

#include <iostream>

using namespace mitk;


class mitkPointTypeConversionTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkPointTypeConversionTestSuite);

  MITK_TEST(Vector2Point);

  MITK_TEST(Mitk2Itk_PointCompatibility);
  MITK_TEST(Itk2Mitk_PointCompatibility);

  MITK_TEST(Vtk2Mitk_PointCompatibility);

  MITK_TEST(Mitk2Pod_PointCompatibility);
  MITK_TEST(Pod2Mitk_PointCompatibility);

  CPPUNIT_TEST_SUITE_END();

private:

  vtkSmartPointer<vtkPoints>   a_vtkPoints;
  ScalarType originalValues[3];
  ScalarType valuesToCopy[3];

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
  void TestForEquality(T1 v1, T2 v2, std::string v1Name, std::string v2Name, ScalarType eps = mitk::eps)
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("\nAssigning " + v2Name  + " to " + v1Name + ":\n both are equal", true, EqualArray(v1, v2, 3, eps));
  }



public:


  void setUp(void)
  {
    FillVector3D(originalValues, 1.0, 2.0, 3.0);
    FillVector3D(valuesToCopy,   4.0, 5.0, 6.0);

    a_vtkPoints = vtkSmartPointer<vtkPoints>::New();
    a_vtkPoints->Initialize();
  }

  void tearDown(void)
  {
 //   a_vtkPoints = NULL;
  }


  void Mitk2Itk_PointCompatibility()
  {
    itk::Point<ScalarType, 3> itkPoint3D = originalValues;
    mitk::Point3D             point3D    = valuesToCopy;

    itkPoint3D = point3D;

    TestForEquality(itkPoint3D, point3D, "itk::Point", "mitk:Point");
  }


  void Itk2Mitk_PointCompatibility()
  {
    mitk::Point3D point3D                = originalValues;
    itk::Point<ScalarType, 3> itkPoint3D = valuesToCopy;

    point3D = itkPoint3D;

    TestForEquality(point3D, itkPoint3D, "mitk:Point", "itk::Point");
  }


  void Vtk2Mitk_PointCompatibility()
  {
    mitk::Point3D point3D = originalValues;
    a_vtkPoints->InsertNextPoint(valuesToCopy);
    double vtkPoint[3];
    a_vtkPoints->GetPoint(0, vtkPoint);

    point3D = vtkPoint;

    TestForEquality(point3D, vtkPoint, "mitk:Point", "vtkPoint");

  }


  void Mitk2Pod_PointCompatibility()
  {
    ScalarType podPoint[] = {1.0, 2.0, 3.0};
    mitk::Point3D point3D = valuesToCopy;

    point3D.ToArray(podPoint);

    TestForEquality(podPoint, point3D, "POD point", "mitk::Point");

  }

  void Pod2Mitk_PointCompatibility()
  {
    itk::Point<double, 3> point3D = originalValues;
    ScalarType podPoint[] = {4.0, 5.0, 6.0};

    point3D = podPoint;

    TestForEquality(point3D, podPoint, "mitk::Point3D", "POD point");
  }


  void Vector2Point()
  {
    itk::Point<double, 3> point3D   = valuesToCopy;
    itk::Vector<double, 3> vector3D = originalValues;

    point3D = vector3D;

    TestForEquality(point3D, vector3D, "mitk::Point", "mitk::Vector");
  }

};


MITK_TEST_SUITE_REGISTRATION(mitkPointTypeConversion)



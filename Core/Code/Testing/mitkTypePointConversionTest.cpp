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
#include "mitkConstants.h"
#include "mitkTypes.h" // for Equal method
#include "mitkPoint.h"
#include "itkPoint.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"

#include <iostream>

using namespace mitk;

static vtkSmartPointer<vtkPoints>   a_vtkPoints = vtkSmartPointer<vtkPoints>::New();

static const ScalarType originalValues[] =  {1.0, 2.0, 3.0};
static const ScalarType valuesToCopy[]   =  {4.0, 5.0, 6.0};

static void Setup(void)
{
  a_vtkPoints->Initialize();
}


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



static void Test_Mitk2Itk_PointCompatibility()
{
  Setup();

  itk::Point<ScalarType, 3> itkPoint3D = originalValues;
  mitk::Point3D             point3D    = valuesToCopy;

  itkPoint3D = point3D;

  TestForEquality(itkPoint3D, point3D, "itk::Point", "mitk:Point");
}


static void Test_Itk2Mitk_PointCompatibility()
{
  Setup();

  mitk::Point3D point3D                = originalValues;
  itk::Point<ScalarType, 3> itkPoint3D = valuesToCopy;

  point3D = itkPoint3D;

  TestForEquality(point3D, itkPoint3D, "mitk:Point", "itk::Point");
}


static void Test_Vtk2Mitk_PointCompatibility()
{
  Setup();

  mitk::Point3D point3D = originalValues;
  a_vtkPoints->InsertNextPoint(valuesToCopy);
  double vtkPoint[3];
  a_vtkPoints->GetPoint(0, vtkPoint);

  point3D = vtkPoint;

  TestForEquality(point3D, vtkPoint, "mitk:Point", "vtkPoint");

}


static void Test_Mitk2Pod_PointCompatibility()
{
  ScalarType podPoint[] = {1.0, 2.0, 3.0};
  mitk::Point3D point3D = valuesToCopy;

  point3D.ToArray(podPoint);

  TestForEquality(podPoint, point3D, "POD point", "mitk::Point");

}

static void Test_Pod2Mitk_PointCompatibility()
{
  itk::Point<double, 3> point3D = originalValues;
  ScalarType podPoint[] = {4.0, 5.0, 6.0};

  point3D = podPoint;

  TestForEquality(point3D, podPoint, "mitk::Point3D", "POD point");
}


static void Test_Point2Vector()
{
  itk::Point<double, 3> point3D   = valuesToCopy;
  itk::Vector<double, 3> vector3D = originalValues;

  point3D = vector3D;

  TestForEquality(point3D, vector3D, "mitk::Point", "mitk::Vector");
}



/**
* Test the conversions from and to mitk point types
*/
int mitkTypePointConversionTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("PointConversionTest")

  Test_Point2Vector();

  Test_Mitk2Itk_PointCompatibility();
  Test_Itk2Mitk_PointCompatibility();

  Test_Vtk2Mitk_PointCompatibility();

  Test_Mitk2Pod_PointCompatibility();
  Test_Pod2Mitk_PointCompatibility();

  MITK_TEST_END()

}

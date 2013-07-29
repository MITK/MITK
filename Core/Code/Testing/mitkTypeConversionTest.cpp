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
#include "mitkVector.h"
#include "itkPoint.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vnl/vnl_vector_ref.h"

#include <iostream>

using namespace mitk;

static mitk::Point3D         point3D;
static itk::Point<ScalarType, 3> itkPoint3D;

static vtkSmartPointer<vtkPoints>   a_vtkPoints = vtkSmartPointer<vtkPoints>::New();

//static vnl_vector_fixed<ScalarType, 3> vnl_vector;

static void Setup(void)
{
  const ScalarType  mitkValues[]= {1.0, 2.0, 3.0}; /* the values for the mitk point */
  const double  othersValues[]  = {4.0, 5.0, 6.0}; /* the values for all other data representations */

  point3D    = mitkValues;
  itkPoint3D = othersValues;

  a_vtkPoints->Initialize();
  a_vtkPoints->InsertNextPoint(othersValues);


  //vnl_vector.copy_in(othersValues2);
}

static void Test_Mitk2Itk_PointCompatibility()
{
  Setup();
  const itk::Point<ScalarType, 3> copiedPoint = point3D;

  MITK_TEST_CONDITION(
       Equal(copiedPoint[0], point3D[0])
    && Equal(copiedPoint[1], point3D[1])
    && Equal(copiedPoint[2], point3D[2]), "mitk point assigned to itk point")
}

static void Test_Itk2Mitk_PointCompatibility()
{
  Setup();

  const mitk::Point3D copiedPoint = itkPoint3D;

  MITK_TEST_CONDITION(
       Equal(copiedPoint[0], itkPoint3D[0])
    && Equal(copiedPoint[1], itkPoint3D[1])
    && Equal(copiedPoint[2], itkPoint3D[2]), "itk point assigned to mitk point")
}



static void Test_Vtk2Mitk_PointCompatibility()
{
  Setup();

  double vtkPoint[3];
  a_vtkPoints->GetPoint(0, vtkPoint);

  //mitk::Point3D copiedPoint = vtkPoint;

  MITK_TEST_CONDITION(false, "this test is passed if there is no compile error in this method when not commented out :)")
}

static void Test_Vnl2Mitk_PointCompatibility()
{
  Setup();

  Point3D copiedPoint;

  //  copiedPoint = vnl2mitk(vnl_vector);

  //MITK_TEST_CONDITION(
  //     Equal(copiedPoint[0], static_cast<ScalarType>(vnl_vector[0]))
  //  && Equal(copiedPoint[1], static_cast<ScalarType>(vnl_vector[1]))
  //  && Equal(copiedPoint[2], static_cast<ScalarType>(vnl_vector[2])), "vnl point assigned to mitk point")
}

static void Test_Mitk2Vnl_PointCompatibility()
{
  Setup();

  //vnl_vector_fixed<ScalarType, 3> copiedPoint;

  //  copiedPoint = mitk2vnl(point3D);

  //MITK_TEST_CONDITION(
  //     Equal(static_cast<ScalarType>(copiedPoint[0]), point3D[0])
  //  && Equal(static_cast<ScalarType>(copiedPoint[1]), point3D[1])
  //  && Equal(static_cast<ScalarType>(copiedPoint[2]), point3D[2]), "mitk point assigned to vnl point")
}

/**
 * Test the conversions from and to mitk types
 */
int mitkTypeConversionTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("TypeConversionTest")

  Test_Mitk2Itk_PointCompatibility();
  Test_Itk2Mitk_PointCompatibility();

  Test_Vtk2Mitk_PointCompatibility();

  Test_Vnl2Mitk_PointCompatibility();
  Test_Mitk2Vnl_PointCompatibility();

  MITK_TEST_END()

}

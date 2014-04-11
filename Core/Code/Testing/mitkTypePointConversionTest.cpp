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

static void Test_Mitk2Itk_PointCompatibility()
{
  Setup();

  itk::Point<ScalarType, 3> itkPoint3D = originalValues;
  mitk::Point3D             point3D    = valuesToCopy;

  itkPoint3D = point3D;

  MITK_TEST_CONDITION(itkPoint3D == point3D, "mitk point assigned to itk point")
  MITK_TEST_CONDITION(itkPoint3D == valuesToCopy, "correct values were assigned")
}


static void Test_Itk2Mitk_PointCompatibility()
{
  Setup();

  mitk::Point3D point3D                = originalValues;
  itk::Point<ScalarType, 3> itkPoint3D = valuesToCopy;

  point3D = itkPoint3D;

  MITK_TEST_CONDITION(point3D == itkPoint3D, "itk point assigned to mitk point")
  MITK_TEST_CONDITION(point3D == valuesToCopy, "correct values were assigned")
}


static void Test_Vtk2Mitk_PointCompatibility()
{
  Setup();

  mitk::Point3D point3D = originalValues;
  a_vtkPoints->InsertNextPoint(valuesToCopy);
  double vtkPoint[3];
  a_vtkPoints->GetPoint(0, vtkPoint);

  point3D = vtkPoint;

  MITK_TEST_CONDITION(point3D == vtkPoint, "vtkPoint assigned to mitk point")
  MITK_TEST_CONDITION(point3D == valuesToCopy, "correct values were assigned")
}

static void Test_Mitk2Vtk_PointCompatibility()
{
  Setup();
  double vtkPoint[3];
  mitk::Point3D point3D = valuesToCopy;

  //a_vtkPoints->InsertNextPoint(point3D.GetAsArray());
 // a_vtkPoints->GetPoint(0, vtkPoint);

  MITK_TEST_CONDITION(point3D == vtkPoint, "MITK point assigned to VTK point")
  MITK_TEST_CONDITION(Equal(vtkPoint[0], valuesToCopy[0])
    && Equal(vtkPoint[1], valuesToCopy[1])
    && Equal(vtkPoint[2], valuesToCopy[2]), "correct values were assigned")
}


static void Test_Mitk2Pod_PointCompatibility()
{
  ScalarType podPoint[] = {1.0, 2.0, 3.0};
  mitk::Point3D point3D = valuesToCopy;

  point3D.CopyToArray(podPoint);

  MITK_TEST_CONDITION(point3D  == podPoint, "MITK point assigned to POD point")
  MITK_TEST_CONDITION(Equal(podPoint[0], valuesToCopy[0])
    && Equal(podPoint[1], valuesToCopy[1])
    && Equal(podPoint[2], valuesToCopy[2]), "correct values were assigned")

}

static void Test_Pod2Mitk_PointCompatibility()
{
  mitk::Point3D point3D = originalValues;
  ScalarType podPoint[] = {4.0, 5.0, 6.0};

  point3D = podPoint;

  MITK_TEST_CONDITION(point3D == podPoint, "POD point assigned to MITK point")
  MITK_TEST_CONDITION(point3D == valuesToCopy, "correct values were assigned")
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
* Test the conversions from and to mitk point types
*/
int mitkTypePointConversionTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("PointConversionTest")

  Test_Mitk2Itk_PointCompatibility();
  Test_Itk2Mitk_PointCompatibility();

  Test_Vtk2Mitk_PointCompatibility();
  Test_Mitk2Vtk_PointCompatibility();

  Test_Mitk2Vnl_PointCompatibility();

  Test_Mitk2Pod_PointCompatibility();
  Test_Pod2Mitk_PointCompatibility();

  MITK_TEST_END()

}

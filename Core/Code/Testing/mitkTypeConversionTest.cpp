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

static vtkSmartPointer<vtkPoints>   a_vtkPoints = vtkSmartPointer<vtkPoints>::New();

static const ScalarType originalValues[] =  {1.0, 2.0, 3.0};
static const ScalarType copiedValues[]   =  {4.0, 5.0, 6.0};

static void Setup(void)
{
  a_vtkPoints->Initialize();
}

static void Test_Mitk2Itk_PointCompatibility()
{
  Setup();

  itk::Point<ScalarType, 3> itkPoint3D = originalValues;
  mitk::Point3D             point3D    = copiedValues;

  itkPoint3D = point3D;

  MITK_TEST_CONDITION(itkPoint3D == point3D, "mitk point assigned to itk point")
  MITK_TEST_CONDITION(itkPoint3D == copiedValues, "correct values were assigned")
}

static void Test_Itk2Mitk_PointCompatibility()
{
  Setup();

  mitk::Point3D point3D                = originalValues;
  itk::Point<ScalarType, 3> itkPoint3D = copiedValues;

  point3D    = itkPoint3D;

  MITK_TEST_CONDITION(point3D == itkPoint3D, "itk point assigned to mitk point")
  MITK_TEST_CONDITION(point3D == copiedValues, "correct values were assigned")
}



static void Test_Vtk2Mitk_PointCompatibility()
{
  Setup();

  mitk::Point3D point3D = originalValues;
  a_vtkPoints->InsertNextPoint(copiedValues);

  double vtkPoint[3];
  a_vtkPoints->GetPoint(0, vtkPoint);

  point3D = vtkPoint;

  MITK_TEST_CONDITION(point3D == vtkPoint, "vtkPoint assigned to mitk point")
  MITK_TEST_CONDITION(point3D == copiedValues, "correct values were assigned")
}


template< class T, unsigned int NVectorDimension>
itk::Vector<T, NVectorDimension>  toItk(vnl_vector_fixed<T, NVectorDimension> vnlVectorFixed)
{
  itk::Vector<T, NVectorDimension> vector;
  vnl_vector<T> vec = vnlVectorFixed.as_vector();

  vector.SetVnlVector(vec);
  return vector;
}


template< class T, unsigned int NVectorDimension>
itk::Vector<T, NVectorDimension>  toItk(vnl_vector<T> vnlVector)
{
  itk::Vector<T, NVectorDimension> vector;
  vector.SetVnlVector(vnlVector);
  return vector;
}


static void Test_Vnl2Mitk_VectorFixedCompatibility()
{
  Setup();

  mitk::Vector3D vector3D = originalValues;
  vnl_vector_fixed<ScalarType, 3> vnlVectorFixed(copiedValues);

  vector3D = toItk(vnlVectorFixed);

  MITK_TEST_CONDITION( vector3D.GetVnlVector() == vnlVectorFixed, "vnl_vector_fixed assigned to mitk vector")
  MITK_TEST_CONDITION( vector3D == copiedValues, "correct values were assigned" )
}

static void Test_Vnl2Mitk_VectorCompatibility()
{
  Setup();

  mitk::Vector3D vector3D = originalValues;
  vnl_vector<ScalarType> vnlVector(3);
  vnlVector.set(copiedValues);

  vector3D = toItk<ScalarType, 3>(vnlVector);

  MITK_TEST_CONDITION( vector3D.GetVnlVector() == vnlVector, "vnl_vector assigned to mitk vector")
  MITK_TEST_CONDITION( vector3D == copiedValues, "correct values were assigned" )
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

  Test_Vnl2Mitk_VectorFixedCompatibility();
  Test_Vnl2Mitk_VectorCompatibility();

  Test_Mitk2Vnl_PointCompatibility();

  MITK_TEST_END()

}

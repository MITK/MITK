/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkImage.h"
#include "mitkVtkAbstractTransformPlaneGeometry.h"
#include "mitkPlaneGeometry.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkSlicedGeometry3D.h"

#include <vtkSphericalTransform.h>

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include <fstream>

int mitkVtkAbstractTransformPlaneGeometryTest(int argc, char* argv[])
{
  mitk::Point3D origin;
  mitk::Vector3D right, bottom;
  mitk::ScalarType width, height;
  mitk::ScalarType widthInMM, heightInMM;

  std::cout << "Initializing an x-/y-plane (xyPlane) as parameter plane by InitializeStandardPlane(rightVector, downVector, spacing = NULL): "<<std::endl;
  mitk::PlaneGeometry::Pointer xyPlane = mitk::PlaneGeometry::New();
  width  = 100;    widthInMM  = width;
  height = 200;    heightInMM = height;
  mitk::ScalarType bounds[6] = {0, width, 0, height, 0, 1};
  mitk::FillVector3D(origin, 4.5,              7.3, 11.2);
  mitk::FillVector3D(right,  widthInMM,          0, 0);
  mitk::FillVector3D(bottom,         0, heightInMM, 0);
  xyPlane->InitializeStandardPlane(right, bottom);
  xyPlane->SetOrigin(origin);
  xyPlane->SetSizeInUnits(width, height);

  std::cout << "Creating VtkAbstractTransformPlaneGeometry: " <<std::endl;
  mitk::VtkAbstractTransformPlaneGeometry::Pointer abstractgeometry=mitk::VtkAbstractTransformPlaneGeometry::New();

  std::cout << "Setting xyPlane as parameter plane of VtkAbstractTransformPlaneGeometry: "<<std::endl;
  abstractgeometry->SetPlane(xyPlane);

  std::cout << "Testing whether the bounds of xyPlane and the parametric bounds of VtkAbstractTransformPlaneGeometry are equal: ";
  if((mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMinimum(), const_cast<mitk::BoundingBox*>(xyPlane->GetBoundingBox())->GetMinimum())==false) || 
     (mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMaximum(), const_cast<mitk::BoundingBox*>(xyPlane->GetBoundingBox())->GetMaximum())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing whether the parametic bounds of VtkAbstractTransformPlaneGeometry and the bounds of the plane accessed from there are equal: ";
  if((mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMinimum(), const_cast<mitk::BoundingBox*>(abstractgeometry->GetPlane()->GetBoundingBox())->GetMinimum())==false) || 
     (mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMaximum(), const_cast<mitk::BoundingBox*>(abstractgeometry->GetPlane()->GetBoundingBox())->GetMaximum())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Change parametic bounds of VtkAbstractTransformPlaneGeometry and test whether they are equal to the bounds of the plane accessed from there: "<<std::endl;
  height = 300;
  bounds[3] = height;
  abstractgeometry->SetParametricBounds(bounds);
  if((mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMinimum(), const_cast<mitk::BoundingBox*>(abstractgeometry->GetPlane()->GetBoundingBox())->GetMinimum())==false) || 
     (mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMaximum(), const_cast<mitk::BoundingBox*>(abstractgeometry->GetPlane()->GetBoundingBox())->GetMaximum())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;



  std::cout << "Initializing an phi-/theta-plane (sphereParameterPlane) as parameter plane by InitializeStandardPlane(rightVector, downVector, spacing = NULL): "<<std::endl;
  mitk::PlaneGeometry::Pointer sphereParameterPlane = mitk::PlaneGeometry::New();
  width  = 100;    widthInMM  = 2*vnl_math::pi;
  height = 200;    heightInMM =   vnl_math::pi;
  mitk::ScalarType radiusInMM = 2.5;

  mitk::FillVector3D(origin, radiusInMM,          0,  widthInMM);
  mitk::FillVector3D(right,  0,                   0, -widthInMM);
  mitk::FillVector3D(bottom, 0,          heightInMM,          0);
  sphereParameterPlane->InitializeStandardPlane(right, bottom);
  sphereParameterPlane->SetOrigin(origin);
  sphereParameterPlane->SetSizeInUnits(width, height);

  std::cout << "Creating an vtkSphericalTransform (sphericalTransform) to use with sphereParameterPlane: "<<std::endl;
  vtkSphericalTransform* sphericalTransform = vtkSphericalTransform::New();

  std::cout << "Setting sphereParameterPlane as parameter plane and sphericalTransform as transform of VtkAbstractTransformPlaneGeometry: "<<std::endl;
  abstractgeometry->SetPlane(sphereParameterPlane);
  abstractgeometry->SetVtkAbstractTransform(sphericalTransform);

  std::cout << "Testing whether the bounds of sphereParameterPlane and the parametric bounds of VtkAbstractTransformPlaneGeometry are equal: ";
  if((mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMinimum(), const_cast<mitk::BoundingBox*>(sphereParameterPlane->GetBoundingBox())->GetMinimum())==false) || 
     (mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMaximum(), const_cast<mitk::BoundingBox*>(sphereParameterPlane->GetBoundingBox())->GetMaximum())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing whether the parametic bounds of VtkAbstractTransformPlaneGeometry and the bounds of the plane accessed from there are equal: ";
  if((mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMinimum(), const_cast<mitk::BoundingBox*>(abstractgeometry->GetPlane()->GetBoundingBox())->GetMinimum())==false) || 
     (mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMaximum(), const_cast<mitk::BoundingBox*>(abstractgeometry->GetPlane()->GetBoundingBox())->GetMaximum())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mapping Map(pt2d_mm(phi=Pi,theta=Pi/2.0), pt3d_mm) and compare with expected (-radius, 0, 0): ";
  mitk::Point2D pt2d_mm;
  mitk::Point3D pt3d_mm, expected_pt3d_mm;
  pt2d_mm[0] = vnl_math::pi; pt2d_mm[1] = vnl_math::pi_over_2;
  mitk::FillVector3D(expected_pt3d_mm, -radiusInMM, 0, 0);
  abstractgeometry->Map(pt2d_mm, pt3d_mm);
  if(mitk::Equal(pt3d_mm, expected_pt3d_mm) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mapping Map(pt3d_mm, pt2d_mm) and compare with expected: ";
  mitk::Point2D testpt2d_mm;
  abstractgeometry->Map(pt3d_mm, testpt2d_mm);
  if(mitk::Equal(pt2d_mm, testpt2d_mm) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing IndexToWorld(pt2d_units, pt2d_mm) and compare with expected: ";
  mitk::Point2D pt2d_units;
  pt2d_units[0] = width/2.0;     pt2d_units[1] = height/2.0;
  pt2d_mm[0]    = widthInMM/2.0; pt2d_mm[1]    = heightInMM/2.0;
  abstractgeometry->IndexToWorld(pt2d_units, testpt2d_mm);
  if(mitk::Equal(pt2d_mm, testpt2d_mm) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Change parametic bounds of VtkAbstractTransformPlaneGeometry and test whether they are equal to the bounds of the plane accessed from there: "<<std::endl;
  height = 300;
  bounds[3] = height;
  abstractgeometry->SetParametricBounds(bounds);
  if((mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMinimum(), const_cast<mitk::BoundingBox*>(abstractgeometry->GetPlane()->GetBoundingBox())->GetMinimum())==false) || 
     (mitk::Equal(const_cast<mitk::BoundingBox*>(abstractgeometry->GetParametricBoundingBox())->GetMaximum(), const_cast<mitk::BoundingBox*>(abstractgeometry->GetPlane()->GetBoundingBox())->GetMaximum())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing IndexToWorld(pt2d_units, pt2d_mm) and compare with expected: ";
  pt2d_units[0] = width/2.0;     pt2d_units[1] = height/2.0;
  pt2d_mm[0]    = widthInMM/2.0; pt2d_mm[1]    = heightInMM/2.0;
  abstractgeometry->IndexToWorld(pt2d_units, testpt2d_mm);
  if(mitk::Equal(pt2d_mm, testpt2d_mm) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  //std::cout << "Testing availability and type (PlaneGeometry) of first geometry in the SlicedGeometry3D: ";
  //mitk::PlaneGeometry* accessedplanegeometry3 = dynamic_cast<mitk::PlaneGeometry*>(slicedWorldGeometry->GetGeometry2D(0));
  //if(accessedplanegeometry3==NULL)
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing whether the first geometry in the SlicedGeometry3D is identical to planegeometry3 by axis comparison: "<<std::endl;
  //if((mitk::Equal(accessedplanegeometry3->GetAxisVector(0), planegeometry3->GetAxisVector(0))==false) || 
  //   (mitk::Equal(accessedplanegeometry3->GetAxisVector(1), planegeometry3->GetAxisVector(1))==false) || 
  //   (mitk::Equal(accessedplanegeometry3->GetAxisVector(2), planegeometry3->GetAxisVector(2))==false))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;


  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}

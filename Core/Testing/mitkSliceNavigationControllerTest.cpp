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

#include "mitkSliceNavigationController.h"
#include "mitkPlaneGeometry.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkTimeSlicedGeometry.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include <fstream>

bool operator==(const mitk::Geometry3D & left, const mitk::Geometry3D & right)
{
  mitk::BoundingBox::BoundsArrayType leftbounds, rightbounds;
  leftbounds =left.GetBounds();
  rightbounds=right.GetBounds();

  unsigned int i;
  for(i=0;i<6;++i)
    if(mitk::Equal(leftbounds[i],rightbounds[i])==false) return false;

  const mitk::Geometry3D::TransformType::MatrixType & leftmatrix  = left.GetIndexToWorldTransform()->GetMatrix();
  const mitk::Geometry3D::TransformType::MatrixType & rightmatrix = right.GetIndexToWorldTransform()->GetMatrix();

  unsigned int j;
  for(i=0;i<3;++i)
  {
    const mitk::Geometry3D::TransformType::MatrixType::ValueType* leftvector  = leftmatrix[i];
    const mitk::Geometry3D::TransformType::MatrixType::ValueType* rightvector = rightmatrix[i];
    for(j=0;j<3;++j)
      if(mitk::Equal(leftvector[i],rightvector[i])==false) return false;
  }

  const mitk::Geometry3D::TransformType::OffsetType & leftoffset  = left.GetIndexToWorldTransform()->GetOffset();
  const mitk::Geometry3D::TransformType::OffsetType & rightoffset = right.GetIndexToWorldTransform()->GetOffset();
  for(i=0;i<3;++i)
    if(mitk::Equal(leftoffset[i],rightoffset[i])==false) return false;
  
  return true;
}

int compareGeometry(const mitk::Geometry3D & geometry, 
                 const mitk::ScalarType& width, const mitk::ScalarType& height, const mitk::ScalarType& numSlices, 
                 const mitk::ScalarType& widthInMM, const mitk::ScalarType& heightInMM, const mitk::ScalarType& thicknessInMM, 
                 const mitk::Point3D& cornerpoint0, const mitk::Vector3D& right, const mitk::Vector3D& bottom, const mitk::Vector3D& normal)
{
  std::cout << "Testing width, height and thickness (in units): ";
  if((mitk::Equal(geometry.GetExtent(0),width)==false) || 
     (mitk::Equal(geometry.GetExtent(1),height)==false) || 
     (mitk::Equal(geometry.GetExtent(2),numSlices)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm): ";
  if((mitk::Equal(geometry.GetExtentInMM(0),widthInMM)==false) || 
     (mitk::Equal(geometry.GetExtentInMM(1),heightInMM)==false) || 
     (mitk::Equal(geometry.GetExtentInMM(2),thicknessInMM)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector(): ";
  std::cout << "dir=0 ";
  mitk::Vector3D dv;
  dv=right; dv.Normalize(); dv*=widthInMM;
  if((mitk::Equal(geometry.GetAxisVector(0), dv)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]";
  std::cout << ", dir=1 ";
  dv=bottom; dv.Normalize(); dv*=heightInMM;
  if((mitk::Equal(geometry.GetAxisVector(1), dv)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]";
  std::cout << ", dir=2 ";
  dv=normal; dv.Normalize(); dv*=thicknessInMM;
  if((mitk::Equal(geometry.GetAxisVector(2), dv)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing offset: ";
  if((mitk::Equal(geometry.GetCornerPoint(0),cornerpoint0)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  return EXIT_SUCCESS;
}

int testGeometry(const mitk::Geometry3D * geometry, 
                 const mitk::ScalarType& width, const mitk::ScalarType& height, const mitk::ScalarType& numSlices,
                 const mitk::ScalarType& widthInMM, const mitk::ScalarType& heightInMM, const mitk::ScalarType& thicknessInMM, 
                 const mitk::Point3D& cornerpoint0, const mitk::Vector3D& right, const mitk::Vector3D& bottom, const mitk::Vector3D& normal)
{
  int result=EXIT_FAILURE;

  std::cout << "Comparing GetCornerPoint(0) of Geometry3D with provided cornerpoint0: ";
  if(mitk::Equal(geometry->GetCornerPoint(0), cornerpoint0)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating and initializing a SliceNavigationController with the Geometry3D: ";
  mitk::SliceNavigationController::Pointer sliceCtrl = mitk::SliceNavigationController::New();
  sliceCtrl->SetInputWorldGeometry(geometry);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing SetViewDirection(mitk::SliceNavigationController::Transversal): ";
  sliceCtrl->SetViewDirection(mitk::SliceNavigationController::Transversal);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing Update(): ";
  sliceCtrl->Update();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing result of CreatedWorldGeometry(): ";
  mitk::Point3D transversalcornerpoint0;
  transversalcornerpoint0 = cornerpoint0+bottom+normal*(numSlices-1+0.5); //really -1?
  result = compareGeometry(*sliceCtrl->GetCreatedWorldGeometry(), width, height, numSlices, widthInMM, heightInMM, thicknessInMM*numSlices, transversalcornerpoint0, right, bottom*(-1.0), normal*(-1.0));
  if(result!=EXIT_SUCCESS)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return result;
  }
  std::cout<<"[PASSED]"<<std::endl;



  std::cout << "Testing SetViewDirection(mitk::SliceNavigationController::Frontal): ";
  sliceCtrl->SetViewDirection(mitk::SliceNavigationController::Frontal);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing Update(): ";
  sliceCtrl->Update();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing result of CreatedWorldGeometry(): ";
  mitk::Point3D frontalcornerpoint0;
  frontalcornerpoint0 = cornerpoint0+geometry->GetAxisVector(1)*(+0.5/geometry->GetExtent(1));
  result = compareGeometry(*sliceCtrl->GetCreatedWorldGeometry(), width, numSlices, height, widthInMM, thicknessInMM*numSlices, heightInMM, frontalcornerpoint0, right, normal, bottom);
  if(result!=EXIT_SUCCESS)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return result;
  }
  std::cout<<"[PASSED]"<<std::endl;



  std::cout << "Testing SetViewDirection(mitk::SliceNavigationController::Sagittal): ";
  sliceCtrl->SetViewDirection(mitk::SliceNavigationController::Sagittal);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing Update(): "<<std::endl;
  sliceCtrl->Update();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing result of CreatedWorldGeometry(): ";
  mitk::Point3D sagittalcornerpoint0;
  sagittalcornerpoint0 = cornerpoint0+geometry->GetAxisVector(0)*(+0.5/geometry->GetExtent(0));
  result = compareGeometry(*sliceCtrl->GetCreatedWorldGeometry(), height, numSlices, width, heightInMM, thicknessInMM*numSlices, widthInMM, sagittalcornerpoint0, bottom, normal, right);
  if(result!=EXIT_SUCCESS)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return result;
  }
  std::cout<<"[PASSED]"<<std::endl;

  return EXIT_SUCCESS;
}

int mitkSliceNavigationControllerTest(int /*argc*/, char* /*argv*/[])
{
  int result=EXIT_FAILURE;

  std::cout << "Creating and initializing a PlaneGeometry: ";
  mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();
 
  mitk::Point3D origin;
  mitk::Vector3D right, bottom, normal;
  mitk::ScalarType width, height;
  mitk::ScalarType widthInMM, heightInMM, thicknessInMM;

  width  = 100;    widthInMM  = width;
  height = 200;    heightInMM = height;
  thicknessInMM = 1.5;
//  mitk::FillVector3D(origin,         0,          0, thicknessInMM*0.5);
  mitk::FillVector3D(origin, 4.5,              7.3, 11.2);
  mitk::FillVector3D(right,  widthInMM,          0, 0);
  mitk::FillVector3D(bottom,         0, heightInMM, 0);
  mitk::FillVector3D(normal,         0,          0, thicknessInMM);

  mitk::Vector3D spacing;
  normal.Normalize(); normal *= thicknessInMM;
  mitk::FillVector3D(spacing, 1.0, 1.0, thicknessInMM);
  planegeometry->InitializeStandardPlane(right.Get_vnl_vector(), bottom.Get_vnl_vector(), &spacing);
  planegeometry->SetOrigin(origin);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating and initializing a SlicedGeometry3D with the PlaneGeometry: ";
  mitk::SlicedGeometry3D::Pointer slicedgeometry = mitk::SlicedGeometry3D::New();
  unsigned int numSlices = 5;
  slicedgeometry->InitializeEvenlySpaced(planegeometry, thicknessInMM, numSlices, false);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Creating a Geometry3D with the same extent as the SlicedGeometry3D: ";
  mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
  geometry->SetBounds(slicedgeometry->GetBounds());
  geometry->SetIndexToWorldTransform(slicedgeometry->GetIndexToWorldTransform());
  std::cout<<"[PASSED]"<<std::endl;

  mitk::Point3D cornerpoint0;
  cornerpoint0 = geometry->GetCornerPoint(0);


  result=testGeometry(geometry, width, height, numSlices, widthInMM, heightInMM, thicknessInMM, cornerpoint0, right, bottom, normal);
  if(result!=EXIT_SUCCESS)
    return result;



  mitk::BoundingBox::BoundsArrayType bounds =  geometry->GetBounds();
  mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
  transform->SetMatrix(geometry->GetIndexToWorldTransform()->GetMatrix());
  mitk::BoundingBox::Pointer boundingbox = geometry->CalculateBoundingBoxRelativeToTransform(transform);
  geometry->SetBounds(boundingbox->GetBounds());
  cornerpoint0 = geometry->GetCornerPoint(0);

  result=testGeometry(geometry, width, height, numSlices, widthInMM, heightInMM, thicknessInMM, cornerpoint0, right, bottom, normal);
  if(result!=EXIT_SUCCESS)
    return result;



  std::cout << "Changing the IndexToWorldTransform of the geometry to a rotated version by SetIndexToWorldTransform() (keep cornerpoint0): ";
  transform = mitk::AffineTransform3D::New();
  mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
  vnlmatrix = planegeometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix();
  mitk::VnlVector axis(3);
  mitk::FillVector3D(axis, 1.0, 1.0, 1.0); axis.normalize();
  vnl_quaternion<mitk::ScalarType> rotation(axis, 0.223);
  vnlmatrix = rotation.rotation_matrix_transpose()*vnlmatrix;
  mitk::Matrix3D matrix;
  matrix = vnlmatrix;
  transform->SetMatrix(matrix);
  transform->SetOffset(cornerpoint0.GetVectorFromOrigin());
  
  right.Set_vnl_vector( rotation.rotation_matrix_transpose()*right.Get_vnl_vector() );
  bottom.Set_vnl_vector(rotation.rotation_matrix_transpose()*bottom.Get_vnl_vector());
  normal.Set_vnl_vector(rotation.rotation_matrix_transpose()*normal.Get_vnl_vector());
  geometry->SetIndexToWorldTransform(transform);
  std::cout<<"[PASSED]"<<std::endl;

  cornerpoint0 = geometry->GetCornerPoint(0);

  result = testGeometry(geometry, width, height, numSlices, widthInMM, heightInMM, thicknessInMM, cornerpoint0, right, bottom, normal);
  if(result!=EXIT_SUCCESS)
    return result;





  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}

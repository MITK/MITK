/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkGeometry3D.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include <fstream>

bool testGetAxisVectorVariants(mitk::Geometry3D* geometry)
{
  int direction;
  for(direction=0; direction<3; ++direction)
  {
    mitk::Vector3D frontToBack;
    switch(direction)
    {
      case 0: frontToBack = geometry->GetCornerPoint(false, false, false)-geometry->GetCornerPoint(true , false, false); break; //7-3
      case 1: frontToBack = geometry->GetCornerPoint(false, false, false)-geometry->GetCornerPoint(false, true , false); break; //7-5
      case 2: frontToBack = geometry->GetCornerPoint(false, false, false)-geometry->GetCornerPoint(false , false, true);  break; //7-2
    }
    std::cout << "Testing GetAxisVector(int) vs GetAxisVector(bool, bool, bool): ";
    if(mitk::Equal((double)geometry->GetAxisVector(direction), (double)frontToBack) == false)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return false;
    }
    std::cout<<"[PASSED]"<<std::endl;
  }
  return true;
}

bool testGetAxisVectorExtent(mitk::Geometry3D* geometry)
{
  int direction;
  for(direction=0; direction<3; ++direction)
  {
    if(mitk::Equal((double)geometry->GetAxisVector(direction).GetNorm(), (double)geometry->GetExtentInMM(direction)) == false)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return false;
    }
    std::cout<<"[PASSED]"<<std::endl;
  }
  return true;
}

int mitkGeometry3DTest(int argc, char* argv[])
{
  float bounds[ ] = {-10.0, 17.0, -12.0, 188.0, 13.0, 211.0};

  mitk::Geometry3D::Pointer geometry3d = mitk::Geometry3D::New();

  std::cout << "Initializing: ";
  geometry3d->Initialize();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Setting bounds by SetFloatBounds(): ";
  geometry3d->SetFloatBounds(bounds);
  std::cout<<"[PASSED]"<<std::endl;
 
  if(testGetAxisVectorVariants(geometry3d) == false)
    return EXIT_FAILURE;

  if(testGetAxisVectorExtent(geometry3d) == false)
    return EXIT_FAILURE;

  //planegeometry->InitializeStandardPlane(right.Get_vnl_vector(), bottom.Get_vnl_vector());

  //std::cout << "Testing width, height and thickness (in units): ";
  //if((planegeometry->GetExtent(0) != width) || (planegeometry->GetExtent(1) != height) || (planegeometry->GetExtent(2) != 1))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing width, height and thickness (in mm): ";
  //if((planegeometry->GetExtentInMM(0) != widthInMM) || (planegeometry->GetExtentInMM(1) != heightInMM) || (planegeometry->GetExtentInMM(2) != thicknessInMM))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing GetAxisVector(): ";
  //if((mitk::Equal(planegeometry->GetAxisVector(0), right)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal)==false))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing SetThickness(), querying by GetExtentInMM(2): ";
  //thicknessInMM = 3.5;
  //planegeometry->SetThickness(thicknessInMM);
  //if(planegeometry->GetExtentInMM(2) != thicknessInMM)
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing SetThickness(), querying by GetAxisVector(2) and comparing to normal: ";
  //normal *= thicknessInMM;
  //planegeometry->SetThickness(thicknessInMM);
  //if(mitk::Equal(planegeometry->GetAxisVector(2), normal)==false)
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Changing the UnitsToMMTransform to a rotated version by SetUnitsToMMTransform(): ";
  //mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
  //mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
  //vnlmatrix = planegeometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix();
  //mitk::VnlVector axis(3);
  //mitk::FillVector3D(axis, 1.0, 1.0, 1.0); axis.normalize();
  //vnl_quaternion<mitk::ScalarType> rotation(axis, 0.123);
  //vnlmatrix = rotation.rotation_matrix_transpose()*vnlmatrix;
  //mitk::Matrix3D matrix;
  //matrix = vnlmatrix;
  //transform->SetMatrix(matrix);
  //transform->SetOffset(planegeometry->GetIndexToWorldTransform()->GetOffset());
  //
  //right.Set_vnl_vector( rotation.rotation_matrix_transpose()*right.Get_vnl_vector() );
  //bottom.Set_vnl_vector(rotation.rotation_matrix_transpose()*bottom.Get_vnl_vector());
  //normal.Set_vnl_vector(rotation.rotation_matrix_transpose()*normal.Get_vnl_vector());
  //planegeometry->SetIndexToWorldTransform(transform);

  //std::cout << "Testing width, height and thickness (in mm) of rotated version: ";
  //if((planegeometry->GetExtentInMM(0) != widthInMM) || (planegeometry->GetExtentInMM(1) != heightInMM) || (planegeometry->GetExtentInMM(2) != thicknessInMM))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing GetAxisVector() of rotated version: ";
  //if((mitk::Equal(planegeometry->GetAxisVector(0), right)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal)==false))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing SetSizeInUnits() of rotated version: ";
  //width  *= 2;
  //height *= 3;
  //planegeometry->SetSizeInUnits(width, height);

  //std::cout << "Testing width, height and thickness (in units): ";
  //if((planegeometry->GetExtent(0) != width) || (planegeometry->GetExtent(1) != height) || (planegeometry->GetExtent(2) != 1))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing width, height and thickness (in mm) of version with changed size in units: ";
  //if((planegeometry->GetExtentInMM(0) != widthInMM) || (planegeometry->GetExtentInMM(1) != heightInMM) || (planegeometry->GetExtentInMM(2) != thicknessInMM))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing GetAxisVector() of version with changed size in units: ";
  //if((mitk::Equal(planegeometry->GetAxisVector(0), right)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal)==false))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing Clone(): ";
  //mitk::PlaneGeometry::Pointer clonedplanegeometry = dynamic_cast<mitk::PlaneGeometry*>(planegeometry->Clone().GetPointer());
  //if((clonedplanegeometry.IsNull()) || (clonedplanegeometry->GetReferenceCount()!=1))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing width, height and thickness (in units) of cloned version: ";
  //if((clonedplanegeometry->GetExtent(0) != width) || (clonedplanegeometry->GetExtent(1) != height) || (clonedplanegeometry->GetExtent(2) != 1))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing width, height and thickness (in mm) of cloned version: ";
  //if((clonedplanegeometry->GetExtentInMM(0) != width) || (clonedplanegeometry->GetExtentInMM(1) != height) || (clonedplanegeometry->GetExtentInMM(2) != thicknessInMM))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing GetAxisVector() of cloned version: ";
  //if((mitk::Equal(clonedplanegeometry->GetAxisVector(0), right)==false) || (mitk::Equal(clonedplanegeometry->GetAxisVector(1), bottom)==false) || (mitk::Equal(clonedplanegeometry->GetAxisVector(2), normal)==false))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing InitializeStandardPlane(clonedplanegeometry, planeorientation = Transversal, zPosition = 0, frontside=true): ";
  //planegeometry->InitializeStandardPlane(clonedplanegeometry);

  //std::cout << "Testing width, height and thickness (in units) of transversally initialized version (should be same as in mm due to unit spacing): ";
  //if((planegeometry->GetExtent(0) != width) || (planegeometry->GetExtent(1) != height) || (planegeometry->GetExtent(2) != thicknessInMM))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing width, height and thickness (in mm) of transversally initialized version: ";
  //if((planegeometry->GetExtentInMM(0) != widthInMM) || (planegeometry->GetExtentInMM(1) != heightInMM) || (planegeometry->GetExtentInMM(2) != thicknessInMM))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  //std::cout << "Testing GetAxisVector() of of transversally initialized version: ";
  //if((mitk::Equal(planegeometry->GetAxisVector(0), right)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal)==false))
  //{
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}

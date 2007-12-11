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
#include "mitkGeometry.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkSlicedGeometry3D.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include <fstream>

int mitkGeometryTest(int argc, char* argv[])
{
  mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();
 
  mitk::Vector3D right, bottom, normal;
  unsigned int width, height;
  mitk::ScalarType widthInMM, heightInMM, thicknessInMM;

  width  = 100;    widthInMM  = width;
  height = 200;    heightInMM = height;
  thicknessInMM = 1.0;
  mitk::FillVector3D(right,  widthInMM,          0, 0);
  mitk::FillVector3D(bottom,         0, heightInMM, 0);
  mitk::FillVector3D(normal,         0,          0, thicknessInMM);


GetAxisVector(direction)== the following!

    //@fixme test
    Vector3D frontToBack2;
    switch(direction)
    {
      case 0: frontToBack = GetCornerPoint(false, false, false)-GetCornerPoint(true , false, false); break; //7-3
      case 1: frontToBack = GetCornerPoint(false, false, false)-GetCornerPoint(false, true , false); break; //7-5
      case 2: frontToBack = GetCornerPoint(false, false, false)-GetCornerPoint(true , false, true);  break; //7-2
      default: assert(direction>=0 && direction<3);
    }
    return frontToBack;

GetAxisVector(direction).GetNorm() ==GetExtentInMM(direction)

  std::cout << "Testing InitializeStandardPlane(rightVector, downVector, spacing = NULL): ";
  planegeometry->InitializeStandardPlane(right.Get_vnl_vector(), bottom.Get_vnl_vector());

  std::cout << "Testing width, height and thickness (in units): ";
  if((planegeometry->GetExtent(0) != width) || (planegeometry->GetExtent(1) != height) || (planegeometry->GetExtent(2) != 1))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm): ";
  if((planegeometry->GetExtentInMM(0) != widthInMM) || (planegeometry->GetExtentInMM(1) != heightInMM) || (planegeometry->GetExtentInMM(2) != thicknessInMM))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector(): ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), right)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing SetThickness(), querying by GetExtentInMM(2): ";
  thicknessInMM = 3.5;
  planegeometry->SetThickness(thicknessInMM);
  if(planegeometry->GetExtentInMM(2) != thicknessInMM)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing SetThickness(), querying by GetAxisVector(2) and comparing to normal: ";
  normal *= thicknessInMM;
  planegeometry->SetThickness(thicknessInMM);
  if(mitk::Equal(planegeometry->GetAxisVector(2), normal)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Changing the IndexToWorldTransform to a rotated version by SetIndexToWorldTransform(): ";
  mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
  mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
  vnlmatrix = planegeometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix();
  mitk::VnlVector axis(3);
  mitk::FillVector3D(axis, 1.0, 1.0, 1.0); axis.normalize();
  vnl_quaternion<mitk::ScalarType> rotation(axis, 0.123);
  vnlmatrix = rotation.rotation_matrix_transpose()*vnlmatrix;
  mitk::Matrix3D matrix;
  matrix = vnlmatrix;
  transform->SetMatrix(matrix);
  transform->SetOffset(planegeometry->GetIndexToWorldTransform()->GetOffset());
  
  right.Set_vnl_vector( rotation.rotation_matrix_transpose()*right.Get_vnl_vector() );
  bottom.Set_vnl_vector(rotation.rotation_matrix_transpose()*bottom.Get_vnl_vector());
  normal.Set_vnl_vector(rotation.rotation_matrix_transpose()*normal.Get_vnl_vector());
  planegeometry->SetIndexToWorldTransform(transform);

  std::cout << "Testing width, height and thickness (in mm) of rotated version: ";
  if((planegeometry->GetExtentInMM(0) != widthInMM) || (planegeometry->GetExtentInMM(1) != heightInMM) || (planegeometry->GetExtentInMM(2) != thicknessInMM))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of rotated version: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), right)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing SetSizeInUnits() of rotated version: ";
  width  *= 2;
  height *= 3;
  planegeometry->SetSizeInUnits(width, height);

  std::cout << "Testing width, height and thickness (in units): ";
  if((planegeometry->GetExtent(0) != width) || (planegeometry->GetExtent(1) != height) || (planegeometry->GetExtent(2) != 1))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of version with changed size in units: ";
  if((planegeometry->GetExtentInMM(0) != widthInMM) || (planegeometry->GetExtentInMM(1) != heightInMM) || (planegeometry->GetExtentInMM(2) != thicknessInMM))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of version with changed size in units: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), right)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing Clone(): ";
  mitk::PlaneGeometry::Pointer clonedplanegeometry = dynamic_cast<mitk::PlaneGeometry*>(planegeometry->Clone().GetPointer());
  if((clonedplanegeometry.IsNull()) || (clonedplanegeometry->GetReferenceCount()!=1))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in units) of cloned version: ";
  if((clonedplanegeometry->GetExtent(0) != width) || (clonedplanegeometry->GetExtent(1) != height) || (clonedplanegeometry->GetExtent(2) != 1))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of cloned version: ";
  if((clonedplanegeometry->GetExtentInMM(0) != width) || (clonedplanegeometry->GetExtentInMM(1) != height) || (clonedplanegeometry->GetExtentInMM(2) != thicknessInMM))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of cloned version: ";
  if((mitk::Equal(clonedplanegeometry->GetAxisVector(0), right)==false) || (mitk::Equal(clonedplanegeometry->GetAxisVector(1), bottom)==false) || (mitk::Equal(clonedplanegeometry->GetAxisVector(2), normal)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing InitializeStandardPlane(clonedplanegeometry, planeorientation = Transversal, zPosition = 0, frontside=true): ";
  planegeometry->InitializeStandardPlane(clonedplanegeometry);

  std::cout << "Testing width, height and thickness (in units) of transversally initialized version (should be same as in mm due to unit spacing): ";
  if((planegeometry->GetExtent(0) != width) || (planegeometry->GetExtent(1) != height) || (planegeometry->GetExtent(2) != thicknessInMM))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of transversally initialized version: ";
  if((planegeometry->GetExtentInMM(0) != widthInMM) || (planegeometry->GetExtentInMM(1) != heightInMM) || (planegeometry->GetExtentInMM(2) != thicknessInMM))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of of transversally initialized version: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), right)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}

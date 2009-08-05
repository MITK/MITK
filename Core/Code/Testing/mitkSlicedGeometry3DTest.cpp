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
#include "mitkPlaneGeometry.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkSlicedGeometry3D.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include <fstream>

int mitkSlicedGeometry3DTest(int /*argc*/, char* /*argv*/[])
{
  mitk::PlaneGeometry::Pointer planegeometry1 = mitk::PlaneGeometry::New();
 
  mitk::Point3D origin;
  mitk::Vector3D right, bottom, normal;
  mitk::ScalarType width, height;
  mitk::ScalarType widthInMM, heightInMM, thicknessInMM;

  width  = 100;    widthInMM  = width;
  height = 200;    heightInMM = height;
  thicknessInMM = 3.5;
  mitk::FillVector3D(origin, 4.5,              7.3, 11.2);
  mitk::FillVector3D(right,  widthInMM,          0, 0);
  mitk::FillVector3D(bottom,         0, heightInMM, 0);
  mitk::FillVector3D(normal,         0,          0, thicknessInMM);

  std::cout << "Initializing planegeometry1 by InitializeStandardPlane(rightVector, downVector, spacing = NULL): "<<std::endl;
  planegeometry1->InitializeStandardPlane(right.Get_vnl_vector(), bottom.Get_vnl_vector());

  std::cout << "Setting planegeometry2 to a cloned version of planegeometry1: "<<std::endl;
  mitk::PlaneGeometry::Pointer planegeometry2;
  planegeometry2 = dynamic_cast<mitk::PlaneGeometry*>(planegeometry1->Clone().GetPointer());;

  std::cout << "Changing the IndexToWorldTransform of planegeometry2 to a rotated version by SetIndexToWorldTransform() (keep origin): "<<std::endl;
  mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
  mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
  vnlmatrix = planegeometry2->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix();
  mitk::VnlVector axis(3);
  mitk::FillVector3D(axis, 1.0, 1.0, 1.0); axis.normalize();
  vnl_quaternion<mitk::ScalarType> rotation(axis, 0.123);
  vnlmatrix = rotation.rotation_matrix_transpose()*vnlmatrix;
  mitk::Matrix3D matrix;
  matrix = vnlmatrix;
  transform->SetMatrix(matrix);
  transform->SetOffset(planegeometry2->GetIndexToWorldTransform()->GetOffset());
  
  right.Set_vnl_vector( rotation.rotation_matrix_transpose()*right.Get_vnl_vector() );
  bottom.Set_vnl_vector(rotation.rotation_matrix_transpose()*bottom.Get_vnl_vector());
  normal.Set_vnl_vector(rotation.rotation_matrix_transpose()*normal.Get_vnl_vector());
  planegeometry2->SetIndexToWorldTransform(transform);


  std::cout << "Setting planegeometry3 to the backside of planegeometry2: " <<std::endl;
  mitk::PlaneGeometry::Pointer planegeometry3 = mitk::PlaneGeometry::New();
  planegeometry3->InitializeStandardPlane(planegeometry2, mitk::PlaneGeometry::Transversal, 0, false);


  std::cout << "Testing SlicedGeometry3D::InitializeEvenlySpaced(planegeometry3, zSpacing = 1, slices = 5, flipped = false): " <<std::endl;
  mitk::SlicedGeometry3D::Pointer slicedWorldGeometry=mitk::SlicedGeometry3D::New();
  unsigned int numSlices = 5;
  slicedWorldGeometry->InitializeEvenlySpaced(planegeometry3, 1, numSlices, false);

  std::cout << "Testing availability and type (PlaneGeometry) of first geometry in the SlicedGeometry3D: ";
  mitk::PlaneGeometry* accessedplanegeometry3 = dynamic_cast<mitk::PlaneGeometry*>(slicedWorldGeometry->GetGeometry2D(0));
  if(accessedplanegeometry3==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing whether the first geometry in the SlicedGeometry3D is identical to planegeometry3 by axis comparison and origin: "<<std::endl;
  if((mitk::Equal(accessedplanegeometry3->GetAxisVector(0), planegeometry3->GetAxisVector(0))==false) || 
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(1), planegeometry3->GetAxisVector(1))==false) || 
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(2), planegeometry3->GetAxisVector(2))==false) ||
     (mitk::Equal(accessedplanegeometry3->GetOrigin(), planegeometry3->GetOrigin())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability and type (PlaneGeometry) of the last geometry in the SlicedGeometry3D: ";
  mitk::PlaneGeometry* accessedplanegeometry3last = dynamic_cast<mitk::PlaneGeometry*>(slicedWorldGeometry->GetGeometry2D(numSlices-1));
  mitk::Point3D origin3last; origin3last = planegeometry3->GetOrigin()+slicedWorldGeometry->GetDirectionVector()*(numSlices-1);
  if(accessedplanegeometry3last==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing whether the last geometry in the SlicedGeometry3D is identical to planegeometry3 by axis comparison: "<<std::endl;
  if((mitk::Equal(accessedplanegeometry3last->GetAxisVector(0), planegeometry3->GetAxisVector(0))==false) || 
     (mitk::Equal(accessedplanegeometry3last->GetAxisVector(1), planegeometry3->GetAxisVector(1))==false) || 
     (mitk::Equal(accessedplanegeometry3last->GetAxisVector(2), planegeometry3->GetAxisVector(2))==false) ||
     (mitk::Equal(accessedplanegeometry3last->GetOrigin(), origin3last)==false) ||
     (mitk::Equal(accessedplanegeometry3last->GetIndexToWorldTransform()->GetOffset(), origin3last.GetVectorFromOrigin())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Again for first slice - Testing availability and type (PlaneGeometry) of first geometry in the SlicedGeometry3D: ";
  accessedplanegeometry3 = dynamic_cast<mitk::PlaneGeometry*>(slicedWorldGeometry->GetGeometry2D(0));
  if(accessedplanegeometry3==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Again for first slice - Testing whether the first geometry in the SlicedGeometry3D is identical to planegeometry3 by axis comparison and origin: "<<std::endl;
  if((mitk::Equal(accessedplanegeometry3->GetAxisVector(0), planegeometry3->GetAxisVector(0))==false) || 
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(1), planegeometry3->GetAxisVector(1))==false) || 
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(2), planegeometry3->GetAxisVector(2))==false) ||
     (mitk::Equal(accessedplanegeometry3->GetOrigin(), planegeometry3->GetOrigin())==false) ||
     (mitk::Equal(accessedplanegeometry3->GetIndexToWorldTransform()->GetOffset(), planegeometry3->GetOrigin().GetVectorFromOrigin())==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}

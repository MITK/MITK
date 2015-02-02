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


#include "mitkImage.h"
#include "mitkPlaneGeometry.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkTestingMacros.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include <fstream>

static const mitk::ScalarType slicedGeometryEps = 1E-9; // epsilon for this testfile. Set to float precision.

void mitkSlicedGeometry3D_ChangeImageGeometryConsideringOriginOffset_Test()
{
  //Tests for Offset
  MITK_TEST_OUTPUT( << "====== NOW RUNNING: Tests for pixel-center-based offset concerns ========");


  // create a SlicedGeometry3D
  mitk::SlicedGeometry3D::Pointer slicedGeo3D=mitk::SlicedGeometry3D::New();
  int num_slices = 5;
  slicedGeo3D->InitializeSlicedGeometry(num_slices); // 5 slices
  mitk::Point3D newOrigin;
  newOrigin[0] = 91.3;
  newOrigin[1] = -13.3;
  newOrigin[2] = 0;
  slicedGeo3D->SetOrigin(newOrigin);
  mitk::Vector3D newSpacing;
  newSpacing[0] = 1.0f;
  newSpacing[1] = 0.9f;
  newSpacing[2] = 0.3f;
  slicedGeo3D->SetSpacing(newSpacing);
  // create subslices as well
  for (int i=0; i < num_slices; i++)
  {
    mitk::PlaneGeometry::Pointer geo2d = mitk::PlaneGeometry::New();
    geo2d->Initialize();
    slicedGeo3D->SetPlaneGeometry(geo2d,i);
  }

  // now run tests

  MITK_TEST_OUTPUT( << "Testing whether slicedGeo3D->GetImageGeometry() is false by default");
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetImageGeometry()==false, "");
  MITK_TEST_OUTPUT( << "Testing whether first and last geometry in the SlicedGeometry3D have GetImageGeometry()==false by default");
  mitk::BaseGeometry* subSliceGeo2D_first = slicedGeo3D->GetPlaneGeometry(0);
  mitk::BaseGeometry* subSliceGeo2D_last = slicedGeo3D->GetPlaneGeometry(num_slices-1);
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetImageGeometry()==false, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetImageGeometry()==false, "");

  // Save some Origins and cornerpoints
  mitk::Point3D OriginSlicedGeo( slicedGeo3D->GetOrigin() );
  mitk::Point3D OriginFirstGeo( subSliceGeo2D_first->GetOrigin() );
  mitk::Point3D OriginLastGeo( subSliceGeo2D_last->GetOrigin() );
  mitk::Point3D CornerPoint0SlicedGeo(slicedGeo3D->GetCornerPoint(0));
  mitk::Point3D CornerPoint1FirstGeo(subSliceGeo2D_first->GetCornerPoint(1));
  mitk::Point3D CornerPoint2LastGeo(subSliceGeo2D_last->GetCornerPoint(2));

  MITK_TEST_OUTPUT( << "Calling slicedGeo3D->ChangeImageGeometryConsideringOriginOffset(true)");
  //std::cout << "vorher Origin: " << subSliceGeo2D_first->GetOrigin() << std::endl;
  //std::cout << "vorher Corner: " << subSliceGeo2D_first->GetCornerPoint(0) << std::endl;
  slicedGeo3D->ChangeImageGeometryConsideringOriginOffset(true);
  //std::cout << "nachher Origin: " << subSliceGeo2D_first->GetOrigin() << std::endl;
  //std::cout << "nachher Corner: " << subSliceGeo2D_first->GetCornerPoint(0) << std::endl;
  MITK_TEST_OUTPUT( << "Testing whether slicedGeo3D->GetImageGeometry() is now true");
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetImageGeometry()==true, "");
  MITK_TEST_OUTPUT( << "Testing whether first and last geometry in the SlicedGeometry3D have GetImageGeometry()==true now");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetImageGeometry()==true, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetImageGeometry()==true, "");

  MITK_TEST_OUTPUT( << "Testing wether offset has been added to origins");
  // Manually adding Offset.
  OriginSlicedGeo[0] += (slicedGeo3D->GetSpacing()[0]) / 2;
  OriginSlicedGeo[1] += (slicedGeo3D->GetSpacing()[1]) / 2;
  OriginSlicedGeo[2] += (slicedGeo3D->GetSpacing()[2]) / 2;
  OriginFirstGeo[0] += (subSliceGeo2D_first->GetSpacing()[0]) / 2;
  OriginFirstGeo[1] += (subSliceGeo2D_first->GetSpacing()[1]) / 2;
  OriginFirstGeo[2] += (subSliceGeo2D_first->GetSpacing()[2]) / 2;
  OriginLastGeo[0] += (subSliceGeo2D_last->GetSpacing()[0]) / 2;
  OriginLastGeo[1] += (subSliceGeo2D_last->GetSpacing()[1]) / 2;
  OriginLastGeo[2] += (subSliceGeo2D_last->GetSpacing()[2]) / 2;
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetCornerPoint(1)==CornerPoint1FirstGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetCornerPoint(2)==CornerPoint2LastGeo, "");
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetCornerPoint(0)==CornerPoint0SlicedGeo, "");
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetOrigin()==OriginSlicedGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetOrigin()==OriginFirstGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetOrigin()==OriginLastGeo, "");

  MITK_TEST_OUTPUT( << "Calling slicedGeo3D->ChangeImageGeometryConsideringOriginOffset(false)");
  slicedGeo3D->ChangeImageGeometryConsideringOriginOffset(false);
  MITK_TEST_OUTPUT( << "Testing whether slicedGeo3D->GetImageGeometry() is now false");
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetImageGeometry()==false, "");
  MITK_TEST_OUTPUT( << "Testing whether first and last geometry in the SlicedGeometry3D have GetImageGeometry()==false now");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetImageGeometry()==false, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetImageGeometry()==false, "");

  MITK_TEST_OUTPUT( << "Testing wether offset has been added to origins of geometry");

  // Manually substracting Offset.
  OriginSlicedGeo[0] -= (slicedGeo3D->GetSpacing()[0]) / 2;
  OriginSlicedGeo[1] -= (slicedGeo3D->GetSpacing()[1]) / 2;
  OriginSlicedGeo[2] -= (slicedGeo3D->GetSpacing()[2]) / 2;
  OriginFirstGeo[0] -= (subSliceGeo2D_first->GetSpacing()[0]) / 2;
  OriginFirstGeo[1] -= (subSliceGeo2D_first->GetSpacing()[1]) / 2;
  OriginFirstGeo[2] -= (subSliceGeo2D_first->GetSpacing()[2]) / 2;
  OriginLastGeo[0] -= (subSliceGeo2D_last->GetSpacing()[0]) / 2;
  OriginLastGeo[1] -= (subSliceGeo2D_last->GetSpacing()[1]) / 2;
  OriginLastGeo[2] -= (subSliceGeo2D_last->GetSpacing()[2]) / 2;

  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetCornerPoint(1)==CornerPoint1FirstGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetCornerPoint(2)==CornerPoint2LastGeo, "");
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetCornerPoint(0)==CornerPoint0SlicedGeo, "");
  MITK_TEST_CONDITION_REQUIRED( slicedGeo3D->GetOrigin()==OriginSlicedGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_first->GetOrigin()==OriginFirstGeo, "");
  MITK_TEST_CONDITION_REQUIRED( subSliceGeo2D_last->GetOrigin()==OriginLastGeo, "");

  MITK_TEST_OUTPUT( << "ALL SUCCESSFULLY!");
}

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
  planegeometry1->InitializeStandardPlane(right.GetVnlVector(), bottom.GetVnlVector());

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

  right.SetVnlVector( rotation.rotation_matrix_transpose()*right.GetVnlVector() );
  bottom.SetVnlVector(rotation.rotation_matrix_transpose()*bottom.GetVnlVector());
  normal.SetVnlVector(rotation.rotation_matrix_transpose()*normal.GetVnlVector());
  planegeometry2->SetIndexToWorldTransform(transform);


  std::cout << "Setting planegeometry3 to the backside of planegeometry2: " <<std::endl;
  mitk::PlaneGeometry::Pointer planegeometry3 = mitk::PlaneGeometry::New();
  planegeometry3->InitializeStandardPlane(planegeometry2, mitk::PlaneGeometry::Axial, 0, false);


  std::cout << "Testing SlicedGeometry3D::InitializeEvenlySpaced(planegeometry3, zSpacing = 1, slices = 5, flipped = false): " <<std::endl;
  mitk::SlicedGeometry3D::Pointer slicedWorldGeometry=mitk::SlicedGeometry3D::New();
  unsigned int numSlices = 5;
  slicedWorldGeometry->InitializeEvenlySpaced(planegeometry3, 1, numSlices, false);

  std::cout << "Testing availability and type (PlaneGeometry) of first geometry in the SlicedGeometry3D: ";
  mitk::PlaneGeometry* accessedplanegeometry3 = dynamic_cast<mitk::PlaneGeometry*>(slicedWorldGeometry->GetPlaneGeometry(0));
  if(accessedplanegeometry3==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing whether the first geometry in the SlicedGeometry3D is identical to planegeometry3 by axis comparison and origin: "<<std::endl;
  if((mitk::Equal(accessedplanegeometry3->GetAxisVector(0), planegeometry3->GetAxisVector(0), slicedGeometryEps)==false) ||
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(1), planegeometry3->GetAxisVector(1), slicedGeometryEps)==false) ||
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(2), planegeometry3->GetAxisVector(2), slicedGeometryEps)==false) ||
     (mitk::Equal(accessedplanegeometry3->GetOrigin(), planegeometry3->GetOrigin(), slicedGeometryEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability and type (PlaneGeometry) of the last geometry in the SlicedGeometry3D: ";
  mitk::PlaneGeometry* accessedplanegeometry3last = dynamic_cast<mitk::PlaneGeometry*>(slicedWorldGeometry->GetPlaneGeometry(numSlices-1));
  mitk::Point3D origin3last; origin3last = planegeometry3->GetOrigin()+slicedWorldGeometry->GetDirectionVector()*(numSlices-1);
  if(accessedplanegeometry3last==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing whether the last geometry in the SlicedGeometry3D is identical to planegeometry3 by axis comparison: "<<std::endl;
  if((mitk::Equal(accessedplanegeometry3last->GetAxisVector(0), planegeometry3->GetAxisVector(0), slicedGeometryEps)==false) ||
     (mitk::Equal(accessedplanegeometry3last->GetAxisVector(1), planegeometry3->GetAxisVector(1), slicedGeometryEps)==false) ||
     (mitk::Equal(accessedplanegeometry3last->GetAxisVector(2), planegeometry3->GetAxisVector(2), slicedGeometryEps)==false) ||
     (mitk::Equal(accessedplanegeometry3last->GetOrigin(), origin3last, slicedGeometryEps)==false) ||
     (mitk::Equal(accessedplanegeometry3last->GetIndexToWorldTransform()->GetOffset(), origin3last.GetVectorFromOrigin(), slicedGeometryEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Again for first slice - Testing availability and type (PlaneGeometry) of first geometry in the SlicedGeometry3D: ";
  accessedplanegeometry3 = dynamic_cast<mitk::PlaneGeometry*>(slicedWorldGeometry->GetPlaneGeometry(0));
  if(accessedplanegeometry3==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Again for first slice - Testing whether the first geometry in the SlicedGeometry3D is identical to planegeometry3 by axis comparison and origin: "<<std::endl;
  if((mitk::Equal(accessedplanegeometry3->GetAxisVector(0), planegeometry3->GetAxisVector(0), slicedGeometryEps)==false) ||
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(1), planegeometry3->GetAxisVector(1), slicedGeometryEps)==false) ||
     (mitk::Equal(accessedplanegeometry3->GetAxisVector(2), planegeometry3->GetAxisVector(2), slicedGeometryEps)==false) ||
     (mitk::Equal(accessedplanegeometry3->GetOrigin(), planegeometry3->GetOrigin(), slicedGeometryEps)==false) ||
     (mitk::Equal(accessedplanegeometry3->GetIndexToWorldTransform()->GetOffset(), planegeometry3->GetOrigin().GetVectorFromOrigin(), slicedGeometryEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  mitkSlicedGeometry3D_ChangeImageGeometryConsideringOriginOffset_Test();

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}

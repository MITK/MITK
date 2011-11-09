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
#include "mitkRotationOperation.h"
#include "mitkInteractionConst.h"
#include "mitkPlanePositionManager.h"
#include "mitkTestingMacros.h"
#include "mitkGetModuleContext.h"

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

int testRestorePlanePostionOperation ()
{
    //Create PlaneGeometry
    mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();

    mitk::Point3D origin;
    mitk::Vector3D right, bottom, normal;
    mitk::ScalarType width, height;
    mitk::ScalarType widthInMM, heightInMM, thicknessInMM;

    width  = 100;    widthInMM  = width;
    height = 200;    heightInMM = height;
    thicknessInMM = 1.5;

    mitk::FillVector3D(origin, 4.5,              7.3, 11.2);
    mitk::FillVector3D(right,  widthInMM,          0, 0);
    mitk::FillVector3D(bottom,         0, heightInMM, 0);
    mitk::FillVector3D(normal,         0,          0, thicknessInMM);

    mitk::Vector3D spacing;
    normal.Normalize(); normal *= thicknessInMM;
    mitk::FillVector3D(spacing, 1.0, 1.0, thicknessInMM);
    planegeometry->InitializeStandardPlane(right.Get_vnl_vector(), bottom.Get_vnl_vector(), &spacing);
    planegeometry->SetOrigin(origin);

    //Create SlicedGeometry3D out of planeGeometry
    mitk::SlicedGeometry3D::Pointer slicedgeometry1 = mitk::SlicedGeometry3D::New();
    unsigned int numSlices = 300;
    slicedgeometry1->InitializeEvenlySpaced(planegeometry, thicknessInMM, numSlices, false);

    //Create another slicedgeo which will be rotated
    mitk::SlicedGeometry3D::Pointer slicedgeometry2 = mitk::SlicedGeometry3D::New();
    slicedgeometry2->InitializeEvenlySpaced(planegeometry, thicknessInMM, numSlices, false);

    //Create  geo3D as reference
    mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
    geometry->SetBounds(slicedgeometry1->GetBounds());
    geometry->SetIndexToWorldTransform(slicedgeometry1->GetIndexToWorldTransform());

    //Initialize planes
    for (int i=0; i < numSlices; i++)
    {
      mitk::PlaneGeometry::Pointer geo2d = mitk::PlaneGeometry::New();
      geo2d->Initialize();
      geo2d->SetReferenceGeometry(geometry);
      slicedgeometry1->SetGeometry2D(geo2d,i);
    }

    for (int i=0; i < numSlices; i++)
    {
      mitk::PlaneGeometry::Pointer geo2d = mitk::PlaneGeometry::New();
      geo2d->Initialize();
      geo2d->SetReferenceGeometry(geometry);
      slicedgeometry2->SetGeometry2D(geo2d,i);
    }

    slicedgeometry1->SetReferenceGeometry(geometry);
    slicedgeometry2->SetReferenceGeometry(geometry);

    //Create SNC
    mitk::SliceNavigationController::Pointer sliceCtrl1 = mitk::SliceNavigationController::New();
    sliceCtrl1->SetInputWorldGeometry(slicedgeometry1);
    sliceCtrl1->Update();

    mitk::SliceNavigationController::Pointer sliceCtrl2 = mitk::SliceNavigationController::New();
    sliceCtrl2->SetInputWorldGeometry(slicedgeometry2);
    sliceCtrl2->Update();

    slicedgeometry1->SetSliceNavigationController(sliceCtrl1);
    slicedgeometry2->SetSliceNavigationController(sliceCtrl2);

    //Rotate slicedgeo2
    double angle = 63.84;
    mitk::Vector3D rotationVector; mitk::FillVector3D( rotationVector, 0.5, 0.95, 0.23 );
    mitk::Point3D center = slicedgeometry2->GetCenter();
    mitk::RotationOperation* op = new mitk::RotationOperation( mitk::OpROTATE, center, rotationVector, angle );
    slicedgeometry2->ExecuteOperation(op);
    sliceCtrl2->Update();

    mitk::ServiceReference serviceRef = mitk::GetModuleContext()->GetServiceReference<mitk::PlanePositionManagerService>();
    mitk::PlanePositionManagerService* service = dynamic_cast<mitk::PlanePositionManagerService*>(mitk::GetModuleContext()->GetService(serviceRef));
    service->AddNewPlanePosition(slicedgeometry2->GetGeometry2D(0), 178);
    sliceCtrl1->ExecuteOperation(service->GetPlanePosition(0));
    sliceCtrl1->Update();
    mitk::Geometry2D* planeRotated = slicedgeometry2->GetGeometry2D(178);
    mitk::Geometry2D* planeRestored = dynamic_cast< const mitk::SlicedGeometry3D*>(sliceCtrl1->GetCurrentGeometry3D())->GetGeometry2D(178);
    bool error = ( !mitk::MatrixEqualElementWise(planeRotated->GetIndexToWorldTransform()->GetMatrix(), planeRestored->GetIndexToWorldTransform()->GetMatrix()) ||
                   !mitk::Equal(planeRotated->GetOrigin(), planeRestored->GetOrigin()) ||
                   !mitk::Equal(planeRotated->GetSpacing(), planeRestored->GetSpacing()) ||
                   !mitk::Equal(slicedgeometry2->GetDirectionVector(), dynamic_cast< const mitk::SlicedGeometry3D*>(sliceCtrl1->GetCurrentGeometry3D())->GetDirectionVector()) ||
                   !mitk::Equal(slicedgeometry2->GetSlices(), dynamic_cast< const mitk::SlicedGeometry3D*>(sliceCtrl1->GetCurrentGeometry3D())->GetSlices()) ||
                   !mitk::MatrixEqualElementWise(slicedgeometry2->GetIndexToWorldTransform()->GetMatrix(), dynamic_cast< const mitk::SlicedGeometry3D*>(sliceCtrl1->GetCurrentGeometry3D())->GetIndexToWorldTransform()->GetMatrix())
                   );

    if (error)
    {
        MITK_TEST_CONDITION(mitk::MatrixEqualElementWise(planeRotated->GetIndexToWorldTransform()->GetMatrix(), planeRestored->GetIndexToWorldTransform()->GetMatrix()),"Testing for IndexToWorld");
        MITK_INFO<<"Rotated: \n"<<planeRotated->GetIndexToWorldTransform()->GetMatrix()<<" Restored: \n"<<planeRestored->GetIndexToWorldTransform()->GetMatrix();
        MITK_TEST_CONDITION(mitk::Equal(planeRotated->GetOrigin(), planeRestored->GetOrigin()),"Testing for origin");
        MITK_INFO<<"Rotated: \n"<<planeRotated->GetOrigin()<<" Restored: \n"<<planeRestored->GetOrigin();
        MITK_TEST_CONDITION(mitk::Equal(planeRotated->GetSpacing(), planeRestored->GetSpacing()),"Testing for spacing");
        MITK_INFO<<"Rotated: \n"<<planeRotated->GetSpacing()<<" Restored: \n"<<planeRestored->GetSpacing();
        MITK_TEST_CONDITION(mitk::Equal(slicedgeometry2->GetDirectionVector(), dynamic_cast< const mitk::SlicedGeometry3D*>(sliceCtrl1->GetCurrentGeometry3D())->GetDirectionVector()),"Testing for directionvector");
        MITK_INFO<<"Rotated: \n"<<slicedgeometry2->GetDirectionVector()<<" Restored: \n"<<dynamic_cast< const mitk::SlicedGeometry3D*>(sliceCtrl1->GetCurrentGeometry3D())->GetDirectionVector();
        MITK_TEST_CONDITION(mitk::Equal(slicedgeometry2->GetSlices(), dynamic_cast< const mitk::SlicedGeometry3D*>(sliceCtrl1->GetCurrentGeometry3D())->GetSlices()),"Testing for numslices");
        MITK_INFO<<"Rotated: \n"<<slicedgeometry2->GetSlices()<<" Restored: \n"<<dynamic_cast< const mitk::SlicedGeometry3D*>(sliceCtrl1->GetCurrentGeometry3D())->GetSlices();
        MITK_TEST_CONDITION(mitk::MatrixEqualElementWise(slicedgeometry2->GetIndexToWorldTransform()->GetMatrix(), dynamic_cast< const mitk::SlicedGeometry3D*>(sliceCtrl1->GetCurrentGeometry3D())->GetIndexToWorldTransform()->GetMatrix()),"Testing for IndexToWorld");
        MITK_INFO<<"Rotated: \n"<<slicedgeometry2->GetIndexToWorldTransform()->GetMatrix()<<" Restored: \n"<<dynamic_cast< const mitk::SlicedGeometry3D*>(sliceCtrl1->GetCurrentGeometry3D())->GetIndexToWorldTransform()->GetMatrix();
        return EXIT_FAILURE;
    }

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

  //Testing Execute RestorePlanePositionOperation
  result = testRestorePlanePostionOperation();
  if(result!=EXIT_SUCCESS)
    return result;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}

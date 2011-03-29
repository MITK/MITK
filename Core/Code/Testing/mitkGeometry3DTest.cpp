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

#include "mitkGeometry3D.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include "mitkRotationOperation.h"
#include "mitkInteractionConst.h"
#include <mitkMatrixConvert.h>

#include "mitkTestingMacros.h"
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
    if(mitk::Equal(geometry->GetAxisVector(direction), frontToBack) == false)
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
    if(mitk::Equal(geometry->GetAxisVector(direction).GetNorm(), geometry->GetExtentInMM(direction)) == false)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return false;
    }
    std::cout<<"[PASSED]"<<std::endl;
  }
  return true;
}

// a part of the test requires axis-parallel coordinates
int testIndexAndWorldConsistency(mitk::Geometry3D* geometry3d)
{
  MITK_TEST_OUTPUT( << "Testing consistency of index and world coordinate systems: ");
  mitk::Point3D origin = geometry3d->GetOrigin();
  mitk::Point3D dummy;

  MITK_TEST_OUTPUT( << " Testing index->world->index conversion consistency");
  geometry3d->WorldToIndex(origin, dummy);
  geometry3d->IndexToWorld(dummy, dummy);
  MITK_TEST_CONDITION_REQUIRED(dummy == origin, "");

  MITK_TEST_OUTPUT( << " Testing WorldToIndex(origin, mitk::Point3D)==(0,0,0)");
  mitk::Point3D globalOrigin;
  mitk::FillVector3D(globalOrigin, 0,0,0);
  
  mitk::Point3D originContinuousIndex;
  geometry3d->WorldToIndex(origin, originContinuousIndex);
  MITK_TEST_CONDITION_REQUIRED(originContinuousIndex == globalOrigin, "");

  MITK_TEST_OUTPUT( << " Testing WorldToIndex(origin, itk::Index)==(0,0,0)");
  itk::Index<3> itkindex;
  geometry3d->WorldToIndex(origin, itkindex);
  itk::Index<3> globalOriginIndex;
  mitk::vtk2itk(globalOrigin, globalOriginIndex);
  MITK_TEST_CONDITION_REQUIRED(itkindex == globalOriginIndex, "");

  MITK_TEST_OUTPUT( << " Testing WorldToIndex(origin-0.5*spacing, itk::Index)==(0,0,0)");
  mitk::Vector3D halfSpacingStep = geometry3d->GetSpacing()*0.5;
  mitk::Matrix3D rotation;
  mitk::Point3D originOffCenter = origin-halfSpacingStep;
  geometry3d->WorldToIndex(originOffCenter, itkindex);
  MITK_TEST_CONDITION_REQUIRED(itkindex == globalOriginIndex, "");

  MITK_TEST_OUTPUT( << " Testing WorldToIndex(origin+0.5*spacing-eps, itk::Index)==(0,0,0)");  
  originOffCenter = origin+halfSpacingStep;
  originOffCenter -= 0.0001;
  geometry3d->WorldToIndex( originOffCenter, itkindex);
  MITK_TEST_CONDITION_REQUIRED(itkindex == globalOriginIndex, "");

  MITK_TEST_OUTPUT( << " Testing WorldToIndex(origin+0.5*spacing, itk::Index)==(1,1,1)");
  originOffCenter = origin+halfSpacingStep;
  itk::Index<3> global111;
  mitk::FillVector3D(global111, 1,1,1);
  geometry3d->WorldToIndex( originOffCenter, itkindex);
  MITK_TEST_CONDITION_REQUIRED(itkindex == global111, "");

  MITK_TEST_OUTPUT( << " Testing WorldToIndex(GetCenter())==BoundingBox.GetCenter: ");
  mitk::Point3D center = geometry3d->GetCenter();
  mitk::Point3D centerContIndex;
  geometry3d->WorldToIndex(center, centerContIndex);
  mitk::BoundingBox::ConstPointer boundingBox = geometry3d->GetBoundingBox();
  mitk::BoundingBox::PointType centerBounds = boundingBox->GetCenter();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(centerContIndex,centerBounds), "");

  MITK_TEST_OUTPUT( << " Testing GetCenter()==IndexToWorld(BoundingBox.GetCenter): ");
  center = geometry3d->GetCenter();
  mitk::Point3D centerBoundsInWorldCoords;
  geometry3d->IndexToWorld(centerBounds, centerBoundsInWorldCoords);
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(center,centerBoundsInWorldCoords), "");

  return EXIT_SUCCESS;
}

int testIndexAndWorldConsistencyForVectors(mitk::Geometry3D* geometry3d)
{
  MITK_TEST_OUTPUT( << "Testing consistency of index and world coordinate systems for vectors: ");
  mitk::Vector3D xAxisMM = geometry3d->GetAxisVector(0);
  mitk::Vector3D xAxisContinuousIndex;
  mitk::Vector3D xAxisContinuousIndexDeprecated;

  mitk::Point3D p, pIndex, origin;
  origin = geometry3d->GetOrigin();
  p[0] = xAxisMM[0];
  p[1] = xAxisMM[1];
  p[2] = xAxisMM[2];

  geometry3d->WorldToIndex(p,pIndex);

  geometry3d->WorldToIndex(origin, xAxisMM, xAxisContinuousIndexDeprecated);
  geometry3d->WorldToIndex(xAxisMM,xAxisContinuousIndex);
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndex[0] == pIndex[0],"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndex[1] == pIndex[1],"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndex[2] == pIndex[2],"");

  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndex[0] == xAxisContinuousIndexDeprecated[0],"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndex[1] == xAxisContinuousIndexDeprecated[1],"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndex[2] == xAxisContinuousIndexDeprecated[2],"");

  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndexDeprecated[0] == pIndex[0],"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndexDeprecated[1] == pIndex[1],"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndexDeprecated[2] == pIndex[2],"");

  
  geometry3d->IndexToWorld(xAxisContinuousIndex,xAxisContinuousIndex);
  geometry3d->IndexToWorld(xAxisContinuousIndexDeprecated,xAxisContinuousIndexDeprecated);
  geometry3d->IndexToWorld(pIndex,p);

  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndex == xAxisMM,"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndex[0] == p[0],"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndex[1] == p[1],"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndex[2] == p[2],"");

  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndexDeprecated == xAxisMM,"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndexDeprecated[0] == p[0],"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndexDeprecated[1] == p[1],"");
  MITK_TEST_CONDITION_REQUIRED(xAxisContinuousIndexDeprecated[2] == p[2],"");

  return EXIT_SUCCESS;
}


#include <itkImage.h>

int testItkImageIsCenterBased()
{
  MITK_TEST_OUTPUT(<< "Testing whether itk::Image coordinates are center-based.");
  typedef itk::Image<int,3> ItkIntImage3D;
  ItkIntImage3D::Pointer itkintimage = ItkIntImage3D::New();
  ItkIntImage3D::SizeType size;
  size.Fill(10);
  mitk::Point3D origin;
  mitk::FillVector3D(origin, 2,3,7);
  itkintimage->Initialize();
  itkintimage->SetRegions(size);
  itkintimage->SetOrigin(origin);
  std::cout<<"[PASSED]"<<std::endl;

  MITK_TEST_OUTPUT( << " Testing itk::Image::TransformPhysicalPointToContinuousIndex(origin)==(0,0,0)");
  mitk::Point3D globalOrigin;
  mitk::FillVector3D(globalOrigin, 0,0,0);

  itk::ContinuousIndex<mitk::ScalarType, 3> originContinuousIndex;
  itkintimage->TransformPhysicalPointToContinuousIndex(origin, originContinuousIndex);
  MITK_TEST_CONDITION_REQUIRED(originContinuousIndex == globalOrigin, "");

  MITK_TEST_OUTPUT( << " Testing itk::Image::TransformPhysicalPointToIndex(origin)==(0,0,0)");
  itk::Index<3> itkindex;
  itkintimage->TransformPhysicalPointToIndex(origin, itkindex);
  itk::Index<3> globalOriginIndex;
  mitk::vtk2itk(globalOrigin, globalOriginIndex);
  MITK_TEST_CONDITION_REQUIRED(itkindex == globalOriginIndex, "");

  MITK_TEST_OUTPUT( << " Testing itk::Image::TransformPhysicalPointToIndex(origin-0.5*spacing)==(0,0,0)");
  mitk::Vector3D halfSpacingStep = itkintimage->GetSpacing()*0.5;
  mitk::Matrix3D rotation;
  mitk::Point3D originOffCenter = origin-halfSpacingStep;
  itkintimage->TransformPhysicalPointToIndex(originOffCenter, itkindex);
  MITK_TEST_CONDITION_REQUIRED(itkindex == globalOriginIndex, "");

  MITK_TEST_OUTPUT( << " Testing itk::Image::TransformPhysicalPointToIndex(origin+0.5*spacing-eps, itk::Index)==(0,0,0)");  
  originOffCenter = origin+halfSpacingStep;
  originOffCenter -= 0.0001;
  itkintimage->TransformPhysicalPointToIndex( originOffCenter, itkindex);
  MITK_TEST_CONDITION_REQUIRED(itkindex == globalOriginIndex, "");

  MITK_TEST_OUTPUT( << " Testing itk::Image::TransformPhysicalPointToIndex(origin+0.5*spacing, itk::Index)==(1,1,1)");
  originOffCenter = origin+halfSpacingStep;
  itk::Index<3> global111;
  mitk::FillVector3D(global111, 1,1,1);
  itkintimage->TransformPhysicalPointToIndex( originOffCenter, itkindex);
  MITK_TEST_CONDITION_REQUIRED(itkindex == global111, "");

  MITK_TEST_OUTPUT( << "=> Yes, itk::Image coordinates are center-based.");

  return EXIT_SUCCESS;
}

int testGeometry3D(bool imageGeometry)
{
  // Build up a new image Geometry    
  mitk::Geometry3D::Pointer geometry3d = mitk::Geometry3D::New();
  float bounds[ ] = {-10.0, 17.0, -12.0, 188.0, 13.0, 211.0};
    
  MITK_TEST_OUTPUT( << "Initializing");
  geometry3d->Initialize();

  MITK_TEST_OUTPUT(<< "Setting ImageGeometry to " << imageGeometry);
  geometry3d->SetImageGeometry(imageGeometry);

  MITK_TEST_OUTPUT(<< "Setting bounds by SetFloatBounds(): " << bounds);
  geometry3d->SetFloatBounds(bounds);
 
  MITK_TEST_OUTPUT( << "Testing AxisVectors");
  if(testGetAxisVectorVariants(geometry3d) == false)
    return EXIT_FAILURE;

  if(testGetAxisVectorExtent(geometry3d) == false)
    return EXIT_FAILURE;

  MITK_TEST_OUTPUT( << "Creating an AffineTransform3D transform");
  mitk::AffineTransform3D::MatrixType matrix;
  matrix.SetIdentity();
  matrix(1,1) = 2;
  mitk::AffineTransform3D::Pointer transform;
  transform = mitk::AffineTransform3D::New();
  transform->SetMatrix(matrix);

  MITK_TEST_OUTPUT( << "Testing a SetIndexToWorldTransform"); 
  geometry3d->SetIndexToWorldTransform(transform);

  MITK_TEST_OUTPUT( << "Testing correctness of value returned by GetSpacing"); 
  const mitk::Vector3D& spacing1 = geometry3d->GetSpacing();
  mitk::Vector3D expectedSpacing;
  expectedSpacing.Fill(1.0);
  expectedSpacing[1] = 2;
  if( mitk::Equal(spacing1, expectedSpacing) == false )
  {
      MITK_TEST_OUTPUT( << " [FAILED]");
      return EXIT_FAILURE;
  }

  MITK_TEST_OUTPUT( << "Testing a Compose(transform)");   
  geometry3d->Compose(transform);

  MITK_TEST_OUTPUT( << "Testing correctness of value returned by GetSpacing");   
  const mitk::Vector3D& spacing2 = geometry3d->GetSpacing();
  expectedSpacing[1] = 4;
  if( mitk::Equal(spacing2, expectedSpacing) == false )
  {
      MITK_TEST_OUTPUT( << " [FAILED]");
      return EXIT_FAILURE;
  }

  MITK_TEST_OUTPUT( << "Testing correctness of SetSpacing");   
  mitk::Vector3D newspacing;
  mitk::FillVector3D(newspacing, 1.5, 2.5, 3.5);
  geometry3d->SetSpacing(newspacing);
  const mitk::Vector3D& spacing3 = geometry3d->GetSpacing();
  if( mitk::Equal(spacing3, newspacing) == false )
  {
      MITK_TEST_OUTPUT( << " [FAILED]");
      return EXIT_FAILURE;
  }

  // Seperate Test function for Index and World consistency
  testIndexAndWorldConsistency(geometry3d);
  testIndexAndWorldConsistencyForVectors(geometry3d);

  MITK_TEST_OUTPUT( << "Testing a rotation of the geometry");   
  double angle = 35.0;
  mitk::Vector3D rotationVector; mitk::FillVector3D( rotationVector, 1, 0, 0 );
  mitk::Point3D center = geometry3d->GetCenter();
  mitk::RotationOperation* op = new mitk::RotationOperation( mitk::OpROTATE, center, rotationVector, angle );
  geometry3d->ExecuteOperation(op);

  MITK_TEST_OUTPUT( << "Testing mitk::GetRotation() and success of rotation");
  mitk::Matrix3D rotation;
  mitk::GetRotation(geometry3d, rotation);
  mitk::Vector3D voxelStep=rotation*newspacing;  
  mitk::Vector3D voxelStepIndex;
  geometry3d->WorldToIndex(geometry3d->GetOrigin(), voxelStep, voxelStepIndex);
  mitk::Vector3D expectedVoxelStepIndex;
  expectedVoxelStepIndex.Fill(1);
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(voxelStepIndex,expectedVoxelStepIndex), "");
  delete op;
  std::cout<<"[PASSED]"<<std::endl;

  MITK_TEST_OUTPUT( << "Testing that ImageGeometry is still " << imageGeometry);
  MITK_TEST_CONDITION_REQUIRED(geometry3d->GetImageGeometry() == imageGeometry, "");

  return EXIT_SUCCESS;
}

int mitkGeometry3DTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkGeometry3DTest);

  int result;

  MITK_TEST_CONDITION_REQUIRED( (result = testItkImageIsCenterBased()) == EXIT_SUCCESS, "");

  MITK_TEST_OUTPUT(<< "Running main part of test with ImageGeometry = false");
  MITK_TEST_CONDITION_REQUIRED( (result = testGeometry3D(false)) == EXIT_SUCCESS, "");

  MITK_TEST_OUTPUT(<< "Running main part of test with ImageGeometry = true");
  MITK_TEST_CONDITION_REQUIRED( (result = testGeometry3D(true)) == EXIT_SUCCESS, "");

  MITK_TEST_END();

  return EXIT_SUCCESS;
}

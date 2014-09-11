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

#include "mitkGeometry3D.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include "mitkRotationOperation.h"
#include "mitkInteractionConst.h"
#include <mitkMatrixConvert.h>
#include <mitkImageCast.h>

#include "mitkTestingMacros.h"
#include <fstream>
#include <mitkNumericTypes.h>

bool testGetAxisVectorVariants(mitk::Geometry3D* geometry)
{
  int direction;
  for(direction=0; direction<3; ++direction)
  {
    mitk::Vector3D frontToBack(0.);
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

  geometry3d->WorldToIndex(xAxisMM, xAxisContinuousIndexDeprecated);
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

int testIndexAndWorldConsistencyForIndex(mitk::Geometry3D* geometry3d)
{
  MITK_TEST_OUTPUT( << "Testing consistency of index and world coordinate systems: ");

  // creating testing data
  itk::Index<4> itkIndex4, itkIndex4b;
  itk::Index<3> itkIndex3, itkIndex3b;
  itk::Index<2> itkIndex2, itkIndex2b;
   itk::Index<3> mitkIndex, mitkIndexb;

  itkIndex4[0] = itkIndex4[1] = itkIndex4[2] = itkIndex4[3] = 4;
  itkIndex3[0] = itkIndex3[1] = itkIndex3[2] = 6;
  itkIndex2[0] = itkIndex2[1] = 2;
  mitkIndex[0] = mitkIndex[1] = mitkIndex[2] = 13;

  // check for constistency
  mitk::Point3D point;
  geometry3d->IndexToWorld(itkIndex2,point);
  geometry3d->WorldToIndex(point,itkIndex2b);

  MITK_TEST_CONDITION_REQUIRED(
    ((itkIndex2b[0] == itkIndex2[0]) &&
    (itkIndex2b[1] == itkIndex2[1])),
    "Testing itk::index<2> for IndexToWorld/WorldToIndex consistency");

  geometry3d->IndexToWorld(itkIndex3,point);
  geometry3d->WorldToIndex(point,itkIndex3b);

  MITK_TEST_CONDITION_REQUIRED(
    ((itkIndex3b[0] == itkIndex3[0]) &&
    (itkIndex3b[1] == itkIndex3[1]) &&
    (itkIndex3b[2] == itkIndex3[2])),
    "Testing itk::index<3> for IndexToWorld/WorldToIndex consistency");

  geometry3d->IndexToWorld(itkIndex4,point);
  geometry3d->WorldToIndex(point,itkIndex4b);

  MITK_TEST_CONDITION_REQUIRED(
    ((itkIndex4b[0] == itkIndex4[0]) &&
    (itkIndex4b[1] == itkIndex4[1]) &&
    (itkIndex4b[2] == itkIndex4[2]) &&
    (itkIndex4b[3] == 0)),
    "Testing itk::index<3> for IndexToWorld/WorldToIndex consistency");

  geometry3d->IndexToWorld(mitkIndex,point);
  geometry3d->WorldToIndex(point,mitkIndexb);

  MITK_TEST_CONDITION_REQUIRED(
    ((mitkIndexb[0] == mitkIndex[0]) &&
    (mitkIndexb[1] == mitkIndex[1]) &&
    (mitkIndexb[2] == mitkIndex[2])),
    "Testing mitk::Index for IndexToWorld/WorldToIndex consistency");

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
  testIndexAndWorldConsistencyForIndex(geometry3d);

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
  geometry3d->WorldToIndex(voxelStep, voxelStepIndex);
  mitk::Vector3D expectedVoxelStepIndex;
  expectedVoxelStepIndex.Fill(1);
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(voxelStepIndex,expectedVoxelStepIndex), "");
  delete op;
  std::cout<<"[PASSED]"<<std::endl;

  MITK_TEST_OUTPUT( << "Testing that ImageGeometry is still " << imageGeometry);
  MITK_TEST_CONDITION_REQUIRED(geometry3d->GetImageGeometry() == imageGeometry, "");

  //Test if the translate function moves the origin correctly.
  mitk::Point3D oldOrigin = geometry3d->GetOrigin();

  //use some random values for translation
  mitk::Vector3D translationVector;
  translationVector.SetElement(0, 17.5f);
  translationVector.SetElement(1, -32.3f);
  translationVector.SetElement(2, 4.0f);
  //compute ground truth
  mitk::Point3D tmpResult = geometry3d->GetOrigin() + translationVector;
  geometry3d->Translate(translationVector);
  MITK_TEST_CONDITION( mitk::Equal( geometry3d->GetOrigin(), tmpResult ), "Testing if origin was translated.");

  translationVector*=-1; //vice versa
  geometry3d->Translate(translationVector);

  MITK_TEST_CONDITION( mitk::Equal( geometry3d->GetOrigin(), oldOrigin ), "Testing if the translation could be done vice versa." );

  return EXIT_SUCCESS;
}

int testGeometryAfterCasting()
{
  // Epsilon. Allowed difference for rotationvalue
  float eps = 0.0001;

  // Cast ITK and MITK images and see if geometry stays
  typedef itk::Image<double,2>  Image2DType;
  typedef itk::Image<double,3>  Image3DType;

  // Create 3D ITK Image from Scratch, cast to 3D MITK image, compare Geometries
  Image3DType::Pointer image3DItk = Image3DType::New();
  Image3DType::RegionType myRegion;
  Image3DType::SizeType mySize;
  Image3DType::IndexType myIndex;
  Image3DType::SpacingType mySpacing;
  Image3DType::DirectionType myDirection, rotMatrixX, rotMatrixY, rotMatrixZ;
  mySpacing[0] = 31;
  mySpacing[1] = 0.1;
  mySpacing[2] = 2.9;
  myIndex[0] = -15;
  myIndex[1] = 15;
  myIndex[2] = 12;
  mySize[0] = 10;
  mySize[1] = 2;
  mySize[2] = 555;
  myRegion.SetSize( mySize);
  myRegion.SetIndex( myIndex );
  image3DItk->SetSpacing(mySpacing);
  image3DItk->SetRegions( myRegion);
  image3DItk->Allocate();
  image3DItk->FillBuffer(0);

  myDirection.SetIdentity();
  rotMatrixX.SetIdentity();
  rotMatrixY.SetIdentity();
  rotMatrixZ.SetIdentity();

  mitk::Image::Pointer mitkImage;

  // direction [row] [coloum]
  MITK_TEST_OUTPUT( << "Casting a rotated 3D ITK Image to a MITK Image and check if Geometry is still same" );
  for (double rotX=0; rotX < (itk::Math::pi*2); rotX+=0.4 )
  {
    // Set Rotation X
    rotMatrixX[1][1] = cos( rotX );
    rotMatrixX[1][2] = -sin( rotX );
    rotMatrixX[2][1] = sin( rotX );
    rotMatrixX[2][2] = cos( rotX );

    for (double rotY=0; rotY < (itk::Math::pi*2); rotY+=0.33 )
    {
      // Set Rotation Y
      rotMatrixY[0][0] = cos( rotY );
      rotMatrixY[0][2] = sin( rotY );
      rotMatrixY[2][0] = -sin( rotY );
      rotMatrixY[2][2] = cos( rotY );

      for (double rotZ=0; rotZ < (itk::Math::pi*2); rotZ+=0.5 )
      {
        // Set Rotation Z
        rotMatrixZ[0][0] = cos( rotZ );
        rotMatrixZ[0][1] = -sin( rotZ );
        rotMatrixZ[1][0] = sin( rotZ );
        rotMatrixZ[1][1] = cos( rotZ );

        // Multiply matrizes
        myDirection = myDirection * rotMatrixX * rotMatrixY * rotMatrixZ;
        image3DItk->SetDirection(myDirection);
        mitk::CastToMitkImage(image3DItk, mitkImage);
        const mitk::AffineTransform3D::MatrixType& matrix = mitkImage->GetGeometry()->GetIndexToWorldTransform()->GetMatrix();

        for (int row=0; row<3; row++)
        {
          for (int col=0; col<3; col++)
          {
            double mitkValue = matrix[row][col] / mitkImage->GetGeometry()->GetSpacing()[col];
            double itkValue  = myDirection[row][col];
            double diff = mitkValue - itkValue;
            // if you decrease this value, you can see that there might be QUITE high inaccuracy!!!
            if (diff > eps) // need to check, how exact it SHOULD be .. since it is NOT EXACT!
            {
              std::cout << "Had a difference of : " << diff;
              std::cout << "Error: Casting altered Geometry!";
              std::cout << "ITK Matrix:\n" << myDirection;
              std::cout << "Mitk Matrix (With Spacing):\n" << matrix;
              std::cout << "Mitk Spacing: " << mitkImage->GetGeometry()->GetSpacing();
              MITK_TEST_CONDITION_REQUIRED(false == true, "");
              return false;
            }
          }
        }
      }
    }
  }

  // Create 2D ITK Image from Scratch, cast to 2D MITK image, compare Geometries
  Image2DType::Pointer image2DItk = Image2DType::New();
  Image2DType::RegionType myRegion2D;
  Image2DType::SizeType mySize2D;
  Image2DType::IndexType myIndex2D;
  Image2DType::SpacingType mySpacing2D;
  Image2DType::DirectionType myDirection2D, rotMatrix;
  mySpacing2D[0] = 31;
  mySpacing2D[1] = 0.1;
  myIndex2D[0] = -15;
  myIndex2D[1] = 15;
  mySize2D[0] = 10;
  mySize2D[1] = 2;
  myRegion2D.SetSize( mySize2D);
  myRegion2D.SetIndex( myIndex2D );
  image2DItk->SetSpacing(mySpacing2D);
  image2DItk->SetRegions( myRegion2D);
  image2DItk->Allocate();
  image2DItk->FillBuffer(0);

  myDirection2D.SetIdentity();
  rotMatrix.SetIdentity();

  // direction [row] [coloum]
  MITK_TEST_OUTPUT( << "Casting a rotated 2D ITK Image to a MITK Image and check if Geometry is still same" );
  for (double rotTheta=0; rotTheta < (itk::Math::pi*2); rotTheta+=0.1 )
  {
    // Set Rotation
    rotMatrix[0][0] = cos(rotTheta);
    rotMatrix[0][1] = -sin(rotTheta);
    rotMatrix[1][0] = sin(rotTheta);
    rotMatrix[1][1] = cos(rotTheta);

    // Multiply matrizes
    myDirection2D = myDirection2D * rotMatrix;
    image2DItk->SetDirection(myDirection2D);
    mitk::CastToMitkImage(image2DItk, mitkImage);
    const mitk::AffineTransform3D::MatrixType& matrix = mitkImage->GetGeometry()->GetIndexToWorldTransform()->GetMatrix();

    // Compare MITK and ITK matrix
    for (int row=0; row<3; row++)
    {
      for (int col=0; col<3; col++)
      {
        double mitkValue = matrix[row][col] / mitkImage->GetGeometry()->GetSpacing()[col];
        if ((row == 2) && (col == row))
        {
          if (mitkValue != 1)
          {
            MITK_TEST_OUTPUT(<< "After casting a 2D ITK to 3D MITK images, MITK matrix values for 0|2, 1|2, 2|0, 2|1 MUST be 0 and value for 2|2 must be 1");
            return false;
          }
        }
        else if ((row == 2) || (col == 2))
        {
          if (mitkValue != 0)
          {
            MITK_TEST_OUTPUT(<< "After casting a 2D ITK to 3D MITK images, MITK matrix values for 0|2, 1|2, 2|0, 2|1 MUST be 0 and value for 2|2 must be 1");
            return false;
          }
        }
        else
        {
          double itkValue  = myDirection2D[row][col];
          double diff = mitkValue - itkValue;
          // if you decrease this value, you can see that there might be QUITE high inaccuracy!!!
          if (diff > eps) // need to check, how exact it SHOULD be .. since it is NOT EXACT!
          {
            std::cout << "Had a difference of : " << diff;
            std::cout << "Error: Casting altered Geometry!";
            std::cout << "ITK Matrix:\n" << myDirection2D;
            std::cout << "Mitk Matrix (With Spacing):\n" << matrix;
            std::cout << "Mitk Spacing: " << mitkImage->GetGeometry()->GetSpacing();
            MITK_TEST_CONDITION_REQUIRED(false == true, "");
            return false;
          }
        }
      }
    }
  }

  // THIS WAS TESTED:
  // 2D ITK -> 2D MITK,
  // 3D ITK -> 3D MITK,

  // Still  need to test: 2D MITK Image with ADDITIONAL INFORMATION IN MATRIX -> 2D ITK
  // 1. Possibility: 3x3 MITK matrix can be converted without loss into 2x2 ITK matrix
  // 2. Possibility: 3x3 MITK matrix can only be converted with loss into 2x2 ITK matrix
  // .. before implementing this, we wait for further development in geometry classes (e.g. Geoemtry3D::SetRotation(..))

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

  MITK_TEST_OUTPUT(<< "Running test to see if Casting MITK to ITK and the other way around destroys geometry");
  MITK_TEST_CONDITION_REQUIRED( (result = testGeometryAfterCasting()) == EXIT_SUCCESS, "");

  MITK_TEST_END();

  return EXIT_SUCCESS;
}

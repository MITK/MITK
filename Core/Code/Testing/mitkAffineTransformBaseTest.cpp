

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


#include "mitkVector.h"
#include "itkScalableAffineTransform.h"
#include "mitkMatrixConvert.h"

#include "mitkTestingMacros.h"

using namespace mitk;

static Vector3D offset;
static Matrix3D rotation;
static Point3D  pointToTransform;

static vtkMatrix4x4* expectedHomogenousMatrix;
static const double  expectedPointAfterTransformation[] = {2, 4, 4, 1};

static void Setup()
{
  pointToTransform[0] = 1.0; pointToTransform[1] = 0.0; pointToTransform[2] = 0.0;

  offset[0] = 2.0; offset[1] = 3.0; offset[2] = 4.0;

  // 90° rotation
  rotation.Fill(0);
  rotation[0][1] = -1;
  rotation[1][0] =  1;

  // set the expected homogenous matrix result
  expectedHomogenousMatrix = vtkMatrix4x4::New();
  expectedHomogenousMatrix->Zero();
  expectedHomogenousMatrix->SetElement(0,1,-1);
  expectedHomogenousMatrix->SetElement(1,0,1);
  expectedHomogenousMatrix->SetElement(0,3,2);
  expectedHomogenousMatrix->SetElement(1,3,3);
  expectedHomogenousMatrix->SetElement(2,3,4);
  expectedHomogenousMatrix->SetElement(3,3,1);
}

/**
* This first test basically assures that we understand the usage of AffineTransform3D correct.
* Meaning that the rotation is set by SetMatrix and the translation is set by SetOffset
*/
static void testIfPointIsTransformedAsExpected(void)
{
  Setup();

  /**
  * create both a AffineTransform3D
  * and let this AffineTransform describe also a homogenous 4x4 Matrix vtkMatrix
  * by using our transfer method
  */
  AffineTransform3D::Pointer transform = AffineTransform3D::New();

  transform->SetOffset(offset);
  transform->SetMatrix(rotation);

  vtkMatrix4x4* homogenMatrix = vtkMatrix4x4::New();
  TransferItkTransformToVtkMatrix(transform.GetPointer(), homogenMatrix);

  // Let a point be transformed by the AffineTransform3D
  Point3D pointTransformedByAffineTransform3D = transform->TransformPoint(pointToTransform);

  bool pointCorrect = true;
  for (int i = 0; i < 3; i++) // only first three since no homogenous coordinates
    pointCorrect &= Equal(pointTransformedByAffineTransform3D[i], expectedPointAfterTransformation[i]);

  MITK_TEST_CONDITION(pointCorrect, "Point has been correctly transformed by AffineTranform3D")
  /*
  // Same Transformation with homogenous matrix
  homogenMatrix->MultiplyDoublePoint
  */

}

/**
* Second test ensures that the function TransferItkTransformToVtkMatrix translates the AffineTransform3D
* correctly to a VtkMatrix4x4
*/
static void testTransferItkTransformToVtkMatrix(void)
{
  Setup();

  AffineTransform3D::Pointer transform = AffineTransform3D::New();

  transform->SetOffset(offset);
  transform->SetMatrix(rotation);

  vtkMatrix4x4* homogenMatrix = vtkMatrix4x4::New();
  TransferItkTransformToVtkMatrix(transform.GetPointer(), homogenMatrix);

  bool allElementsEqual = true;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      allElementsEqual &= Equal(homogenMatrix->GetElement(i,j), expectedHomogenousMatrix->GetElement(i,j));

  MITK_TEST_CONDITION(allElementsEqual, "Homogenous Matrix is set as Expected")
}

/**
 * This test shall ensure and document the basic functionality of the
 * itk AffineTransformation functionality and test some basic transformation functionalities provided by mitk.
 */
int mitkAffineTransformBaseTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("AffineTransformationBaseTest");

  testIfPointIsTransformedAsExpected();

  testTransferItkTransformToVtkMatrix();


  MITK_TEST_END();
}

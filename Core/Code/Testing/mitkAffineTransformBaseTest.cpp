

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


#include "mitkNumericTypes.h"
#include "itkScalableAffineTransform.h"
#include "mitkMatrixConvert.h"

#include "mitkTestingMacros.h"

using namespace mitk;

static Vector3D offset;
static Matrix3D rotation;
static Point3D  originalPoint;
static double   originalPointDouble[4];

static vtkMatrix4x4* homogenMatrix = 0;


static vtkMatrix4x4* expectedHomogenousMatrix = 0;
static const double  expectedPointAfterTransformation[] = {2, 4, 4, 1};

static void Setup()
{
  originalPoint[0] = 1.0; originalPoint[1] = 0.0; originalPoint[2] = 0.0;

  for (int i = 0; i < 3; i++)
    originalPointDouble[i] = originalPoint[i]; // same point as the Point3D version
  originalPointDouble[3] = 1; // homogenous extension

  offset[0] = 2.0; offset[1] = 3.0; offset[2] = 4.0;

  // 90� rotation
  rotation.Fill(0);
  rotation[0][1] = -1;
  rotation[1][0] =  1;

  // prepare a Matrix which shall be set later to a specific
  // homogen matrix by TransferItkTransformToVtkMatrix
  // this makes sure, that the initialization to the identity does not
  // overshadow any bugs in TransferItkTransformToVtkMatrix
  // (it actually did that by "helping out" TransferItkTransformToVtkMatrix
  // by setting the (3,3) element to 1).
  homogenMatrix = vtkMatrix4x4::New();
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      homogenMatrix->SetElement(i,j, i+j*j); // just some not trivial value

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

static void TearDown()
{
  if (homogenMatrix) homogenMatrix->Delete();
  if (expectedHomogenousMatrix) expectedHomogenousMatrix->Delete();
}

/**
* This first test basically assures that we understand the usage of AffineTransform3D correct.
* Meaning that the rotation is set by SetMatrix and the translation is set by SetOffset
*/
static void testIfPointIsTransformedAsExpected(void)
{
  Setup();

  /** construct the transformation */
  AffineTransform3D::Pointer transform = AffineTransform3D::New();

  transform->SetOffset(offset);
  transform->SetMatrix(rotation);

  TransferItkTransformToVtkMatrix(transform.GetPointer(), homogenMatrix);

  /** Let a point be transformed by the AffineTransform3D */
  Point3D pointTransformedByAffineTransform3D = transform->TransformPoint(originalPoint);

  /** assert that the transformation was successful */
  bool pointCorrect = true;
  for (int i = 0; i < 3; i++) // only first three since no homogenous coordinates
    pointCorrect &= Equal(pointTransformedByAffineTransform3D[i], expectedPointAfterTransformation[i]);

  MITK_TEST_CONDITION(pointCorrect, "Point has been correctly transformed by AffineTranform3D")

  TearDown();
}

/**
* This test ensures that the function TransferItkTransformToVtkMatrix translates the AffineTransform3D
* correctly to a VtkMatrix4x4
*/
static void testTransferItkTransformToVtkMatrix(void)
{
  Setup();

  AffineTransform3D::Pointer transform = AffineTransform3D::New();

  transform->SetOffset(offset);
  transform->SetMatrix(rotation);


  TransferItkTransformToVtkMatrix(transform.GetPointer(), homogenMatrix);

  bool allElementsEqual = true;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      allElementsEqual &= Equal(homogenMatrix->GetElement(i,j), expectedHomogenousMatrix->GetElement(i,j));

  MITK_TEST_CONDITION(allElementsEqual, "Homogenous Matrix is set as expected")

  TearDown();
}

/**
 * This test basically is just a sanity check and should be PASSED exactly when both
 * testTransferItkTransformToVtkMatrix and testIfPointIsTransformedAsExpected are PASSED.
 * Tests if we get the same
 * result by using the AffineTransform3D to transform a point or the vtkMatrix4x4 which we got by applying
 * the TransferItkTransformToVtkMatrix function. This test e.g. ensures we made no mistake in our
 * reference results.
 *
 */
static void testIfBothTransformationsProduceSameResults(void)
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

  TransferItkTransformToVtkMatrix(transform.GetPointer(), homogenMatrix);

  /** Let a point be transformed by the AffineTransform3D and by homogenMatrix */
  Point3D pointTransformedByAffineTransform3D = transform->TransformPoint(originalPoint);

  double* pointTransformedByHomogenous= homogenMatrix->MultiplyDoublePoint(originalPointDouble);

  /* check if results match */

  bool pointsMatch = true;
  for (int i = 0; i < 3; i++) // only first three since no homogenous coordinates
    pointsMatch &= Equal(pointTransformedByAffineTransform3D[i], pointTransformedByHomogenous[i]);

  bool homogenousComponentCorrect = Equal(1, pointTransformedByHomogenous[3]);

  MITK_TEST_CONDITION(pointsMatch
    && homogenousComponentCorrect, "Point transformed by AffineTransform and homogenous coordinates match")

  TearDown();
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

  testIfBothTransformationsProduceSameResults();

  MITK_TEST_END();
}

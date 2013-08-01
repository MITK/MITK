

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

static void Setup(Point3D& point3D, Vector3D& offset, Matrix3D& matrix)
{
  point3D[0] = 1.0; point3D[1] = 0.0; point3D[2] = 0.0;

  offset[0] = 2.0; offset[1] = 3.0; offset[2] = 4.0;

  // 90° rotation
  matrix.Fill(0);
  matrix[0][1] = -1;
  matrix[1][0] =  1;


}


static void testTransferItkTransformToVtkMatrix(void)
{
  Vector3D offset;
  Matrix3D rotation;
  Point3D pointToRotate;

  Setup(pointToRotate, offset, rotation);

  AffineTransform3D::Pointer transform = AffineTransform3D::New();

  transform->SetOffset(offset);
  transform->SetMatrix(rotation);

  vtkMatrix4x4* vtkMatrix = vtkMatrix4x4::New();
  TransferItkTransformToVtkMatrix(transform.GetPointer(), vtkMatrix);

  //MITK_TEST_CONDITION(vtkMatrix == )
}

/**
 * This test shall ensure and document the basic functionality of the
 * itk AffineTransformation functionality and test some basic transformation functionalities provided by mitk.
 */
int mitkAffineTransformBaseTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("AffineTransformationBaseTest");

  testTransferItkTransformToVtkMatrix();

  MITK_TEST_END();
}

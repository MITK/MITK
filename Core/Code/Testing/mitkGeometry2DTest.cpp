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
#include <mitkVector.h>


  const mitk::Geometry2D::Pointer  createGeometry2D()
{
   mitk::Vector3D mySpacing;
   mySpacing[0] = 31;
   mySpacing[1] = 0.1;
   mySpacing[2] = 5.4;
   mitk::Point3D myOrigin;
   myOrigin[0] = 8;
   myOrigin[1] = 9;
   myOrigin[2] = 10;
   mitk::AffineTransform3D::Pointer myTransform = mitk::AffineTransform3D::New();
   itk::Matrix<mitk::ScalarType, 3,3> transMatrix;
   transMatrix.Fill(0);
   transMatrix[0][0] = 1;
   transMatrix[1][1] = 2;
   transMatrix[2][2] = 4;

   myTransform->SetMatrix(transMatrix);

   mitk::Geometry2D::Pointer geometry2D = mitk::Geometry2D::New();
   geometry2D->SetIndexToWorldTransform(myTransform);
   geometry2D->SetSpacing(mySpacing);
   geometry2D->SetOrigin(myOrigin);
   return geometry2D;
}

  const int testGeometry2DCloning()
{
  mitk::Geometry2D::Pointer geometry2D = createGeometry2D();

  try
  {
    mitk::Geometry2D::Pointer clone = geometry2D->Clone();
    itk::Matrix<mitk::ScalarType,3,3> matrix = clone->GetIndexToWorldTransform()->GetMatrix();
    MITK_TEST_CONDITION(matrix[0][0] == 31, "Test if matrix element exists...");

    double origin = geometry2D->GetOrigin()[0];
    mitk::Point3D ori = geometry2D->GetOrigin();
    MITK_TEST_CONDITION(mitk::Equal(origin, 8),"First Point of origin as expected...");

    double spacing = geometry2D->GetSpacing()[0];
    mitk::Vector3D spac = geometry2D->GetSpacing();
    MITK_TEST_CONDITION(mitk::Equal(spacing, 31),"First Point of spacing as expected...");
  }
  catch (...)
  {
    MITK_TEST_CONDITION(false, "Error during access on a member of cloned geometry");
  }
   // direction [row] [coloum]
   MITK_TEST_OUTPUT( << "Casting a rotated 2D ITK Image to a MITK Image and check if Geometry is still same" );

  return EXIT_SUCCESS;
}

const bool compareMatrix(itk::Matrix<mitk::ScalarType, 3,3> left, itk::Matrix<mitk::ScalarType, 3,3> right)
{
  bool equal = true;
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      equal &= mitk::Equal(left[i][j], right[i][j]);
  return equal;
}

const int testGeometry2DInitializeOrder()
{
   mitk::Vector3D mySpacing;
   mySpacing[0] = 31;
   mySpacing[1] = 0.1;
   mySpacing[2] = 5.4;
   mitk::Point3D myOrigin;
   myOrigin[0] = 8;
   myOrigin[1] = 9;
   myOrigin[2] = 10;
   mitk::AffineTransform3D::Pointer myTransform = mitk::AffineTransform3D::New();
   itk::Matrix<mitk::ScalarType, 3,3> transMatrix;
   transMatrix.Fill(0);
   transMatrix[0][0] = 1;
   transMatrix[1][1] = 2;
   transMatrix[2][2] = 4;

   myTransform->SetMatrix(transMatrix);

   mitk::Geometry2D::Pointer geometry2D1 = mitk::Geometry2D::New();
   geometry2D1->SetIndexToWorldTransform(myTransform);
   geometry2D1->SetSpacing(mySpacing);
   geometry2D1->SetOrigin(myOrigin);

   mitk::Geometry2D::Pointer geometry2D2 = mitk::Geometry2D::New();
   geometry2D2->SetSpacing(mySpacing);
   geometry2D2->SetOrigin(myOrigin);
   geometry2D2->SetIndexToWorldTransform(myTransform);

   mitk::Geometry2D::Pointer geometry2D3 = mitk::Geometry2D::New();
   geometry2D3->SetIndexToWorldTransform(myTransform);
   geometry2D3->SetSpacing(mySpacing);
   geometry2D3->SetOrigin(myOrigin);
   geometry2D3->SetIndexToWorldTransform(myTransform);

   MITK_TEST_CONDITION(mitk::Equal(geometry2D1->GetOrigin(), geometry2D2->GetOrigin()),"Origin of Geometry 1 match those of Geometry 2.");
   MITK_TEST_CONDITION(mitk::Equal(geometry2D1->GetOrigin(), geometry2D3->GetOrigin()),"Origin of Geometry 1 match those of Geometry 3.");
   MITK_TEST_CONDITION(mitk::Equal(geometry2D2->GetOrigin(), geometry2D3->GetOrigin()),"Origin of Geometry 2 match those of Geometry 3.");

   MITK_TEST_CONDITION(mitk::Equal(geometry2D1->GetSpacing(), geometry2D2->GetSpacing()),"Spacing of Geometry 1 match those of Geometry 2.");
   MITK_TEST_CONDITION(mitk::Equal(geometry2D1->GetSpacing(), geometry2D3->GetSpacing()),"Spacing of Geometry 1 match those of Geometry 3.");
   MITK_TEST_CONDITION(mitk::Equal(geometry2D2->GetSpacing(), geometry2D3->GetSpacing()),"Spacing of Geometry 2 match those of Geometry 3.");

   MITK_TEST_CONDITION(compareMatrix(geometry2D1->GetIndexToWorldTransform()->GetMatrix(), geometry2D2->GetIndexToWorldTransform()->GetMatrix()),"Transformation of Geometry 1 match those of Geometry 2.");
   MITK_TEST_CONDITION(compareMatrix(geometry2D1->GetIndexToWorldTransform()->GetMatrix(), geometry2D3->GetIndexToWorldTransform()->GetMatrix()),"Transformation of Geometry 1 match those of Geometry 3.");
   MITK_TEST_CONDITION(compareMatrix(geometry2D2->GetIndexToWorldTransform()->GetMatrix(), geometry2D3->GetIndexToWorldTransform()->GetMatrix()),"Transformation of Geometry 2 match those of Geometry 3.");
   return EXIT_SUCCESS;
}

int mitkGeometry2DTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkGeometry3DTest);

  int result;

  MITK_TEST_CONDITION_REQUIRED ( (result = testGeometry2DCloning()) == EXIT_SUCCESS, "");

  // See bug 15990
  // MITK_TEST_CONDITION_REQUIRED ( (result = testGeometry2DInitializeOrder()) == EXIT_SUCCESS, "");


  MITK_TEST_END();

  return EXIT_SUCCESS;
}
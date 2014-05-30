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


#include "mitkPlaneGeometry.h"
#include "mitkRotationOperation.h"
#include "mitkInteractionConst.h"
#include "mitkLine.h"

#include "mitkTestingMacros.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

#include <fstream>
#include <iomanip>

static const mitk::ScalarType testEps = 1E-9; // the epsilon used in this test == at least float precision.

int mappingTests2D(const mitk::PlaneGeometry* planegeometry, const mitk::ScalarType& width, const mitk::ScalarType& height, const mitk::ScalarType& widthInMM, const mitk::ScalarType& heightInMM, const mitk::Point3D& origin, const mitk::Vector3D& right, const mitk::Vector3D& bottom)
{

  std::cout << "Testing mapping Map(pt2d_mm(x=widthInMM/2.3,y=heightInMM/2.5), pt3d_mm) and compare with expected: ";
  mitk::Point2D pt2d_mm;
  mitk::Point3D pt3d_mm, expected_pt3d_mm;
  pt2d_mm[0] = widthInMM/2.3; pt2d_mm[1] = heightInMM/2.5;
  expected_pt3d_mm = origin+right*(pt2d_mm[0]/right.GetNorm())+bottom*(pt2d_mm[1]/bottom.GetNorm());
  planegeometry->Map(pt2d_mm, pt3d_mm);
  if(mitk::Equal(pt3d_mm, expected_pt3d_mm, testEps) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mapping Map(pt3d_mm, pt2d_mm) and compare with expected: ";
  mitk::Point2D testpt2d_mm;
  planegeometry->Map(pt3d_mm, testpt2d_mm);
  std::cout << std::setprecision(12) << "Expected pt2d_mm " << pt2d_mm << std::endl;
  std::cout << std::setprecision(12) << "Result testpt2d_mm " << testpt2d_mm << std::endl;
  std::cout << std::setprecision(12) << "10*mitk::eps " << 10*mitk::eps << std::endl;
  //This eps is temporarily set to 10*mitk::eps. See bug #15037 for details.
  if(mitk::Equal(pt2d_mm, testpt2d_mm, 10*mitk::eps) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing IndexToWorld(pt2d_units, pt2d_mm) and compare with expected: ";
  mitk::Point2D pt2d_units;
  pt2d_units[0] = width/2.0;     pt2d_units[1] = height/2.0;
  pt2d_mm[0]    = widthInMM/2.0; pt2d_mm[1]    = heightInMM/2.0;
  planegeometry->IndexToWorld(pt2d_units, testpt2d_mm);

  std::cout << std::setprecision(12) << "Expected pt2d_mm " << pt2d_mm << std::endl;
  std::cout << std::setprecision(12) << "Result testpt2d_mm " << testpt2d_mm << std::endl;
  std::cout << std::setprecision(12) << "10*mitk::eps " << 10*mitk::eps << std::endl;
  //This eps is temporarily set to 10*mitk::eps. See bug #15037 for details.
  if(mitk::Equal(pt2d_mm, testpt2d_mm, 10*mitk::eps) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing WorldToIndex(pt2d_mm, pt2d_units) and compare with expected: ";
  mitk::Point2D testpt2d_units;
  planegeometry->WorldToIndex(pt2d_mm, testpt2d_units);

  std::cout << std::setprecision(12) << "Expected pt2d_units " << pt2d_units << std::endl;
  std::cout << std::setprecision(12) << "Result testpt2d_units " << testpt2d_units << std::endl;
  std::cout << std::setprecision(12) << "10*mitk::eps " << 10*mitk::eps << std::endl;
  //This eps is temporarily set to 10*mitk::eps. See bug #15037 for details.
  if(mitk::Equal(pt2d_units, testpt2d_units, 10*mitk::eps) == false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  return EXIT_SUCCESS;
}

int TestCase1210()
{
  mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();

  mitk::Point3D origin;
  mitk::Vector3D right, down, spacing;

  mitk::FillVector3D(origin, 4.5,              7.3, 11.2);
  mitk::FillVector3D(right,
      1.015625, 1.015625, 1.1999969482421875
      );

  mitk::FillVector3D(down,
      1.4012984643248170709237295832899161312802619418765e-45, 0, 0
      );
  mitk::FillVector3D(spacing,
      0, 1.4713633875410579244699160624544119378442750389703e-43, 9.2806360452222355258639080851310540729807238879469e-32
      );

  std::cout << "Testing InitializeStandardPlane(rightVector, downVector, spacing = NULL): "<<std::endl;
  planegeometry->InitializeStandardPlane(right, down, &spacing);
/*
  std::cout << "Testing width, height and thickness (in units): ";
  if((mitk::Equal(planegeometry->GetExtent(0),width)==false) ||
     (mitk::Equal(planegeometry->GetExtent(1),height)==false) ||
     (mitk::Equal(planegeometry->GetExtent(2),1)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm): ";
  if((mitk::Equal(planegeometry->GetExtentInMM(0),widthInMM)==false) ||
     (mitk::Equal(planegeometry->GetExtentInMM(1),heightInMM)==false) ||
     (mitk::Equal(planegeometry->GetExtentInMM(2),thicknessInMM)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
*/
  std::cout<<"[PASSED]"<<std::endl;

  return EXIT_SUCCESS;
}



/**
 * @brief This method tests method IntersectionPoint
 *
 * See also bug #7151. (ref 2 this test: iggy)
 * This test was written due to incorrect calculation of the intersection point
 * between a given line and plane. This only occured when the pointdistance of
 * the line was less than 1.
 * Test Behavour:
 * ==============
 * we have a given line and a given plane.
 * we let the line intersect the plane.
 * when testing several positions on the line the resulting intersection point must be the same
 * we test a position where the distance between the correspoinding points is < 0 and another position where the distance is > 0.
 *
 */
int TestIntersectionPoint()
{
    //init plane with its parameter
    mitk::PlaneGeometry::Pointer myPlaneGeometry = mitk::PlaneGeometry::New();

    mitk::Point3D origin;
    origin[0] = 0.0;
    origin[1] = 2.0;
    origin[2] = 0.0;

    mitk::Vector3D normal;
    normal[0] = 0.0;
    normal[1] = 1.0;
    normal[2] = 0.0;

    myPlaneGeometry->InitializePlane(origin,normal);

    //generate points and line for intersection testing
    //point distance of given line > 1
    mitk::Point3D pointP1;
    pointP1[0] = 2.0;
    pointP1[1] = 1.0;
    pointP1[2] = 0.0;

    mitk::Point3D pointP2;
    pointP2[0] = 2.0;
    pointP2[1] = 4.0;
    pointP2[2] = 0.0;

    mitk::Vector3D lineDirection;
    lineDirection[0] = pointP2[0] - pointP1[0];
    lineDirection[1] = pointP2[1] - pointP1[1];
    lineDirection[2] = pointP2[2] - pointP1[2];

    mitk::Line3D xingline( pointP1, lineDirection );
    mitk::Point3D calcXingPoint;
    myPlaneGeometry->IntersectionPoint(xingline, calcXingPoint);

    //point distance of given line < 1
    mitk::Point3D pointP3;
    pointP3[0] = 2.0;
    pointP3[1] = 2.2;
    pointP3[2] = 0.0;

    mitk::Point3D pointP4;
    pointP4[0] = 2.0;
    pointP4[1] = 1.7;
    pointP4[2] = 0.0;

    mitk::Vector3D lineDirection2;
    lineDirection2[0] = pointP4[0] - pointP3[0];
    lineDirection2[1] = pointP4[1] - pointP3[1];
    lineDirection2[2] = pointP4[2] - pointP3[2];

    mitk::Line3D xingline2( pointP3, lineDirection2 );
    mitk::Point3D calcXingPoint2;
    myPlaneGeometry->IntersectionPoint( xingline2, calcXingPoint2 );

    //intersection points must be the same
    if (calcXingPoint == calcXingPoint2) {
        return EXIT_SUCCESS;

    } else {
        return EXIT_FAILURE;
    }

}


/**
 * @brief This method tests method ProjectPointOntoPlane.
 *
 * See also bug #3409.
 */
int TestProjectPointOntoPlane()
{
  mitk::PlaneGeometry::Pointer myPlaneGeometry = mitk::PlaneGeometry::New();

  //create normal
  mitk::Vector3D normal;
  normal[0] = 0.0;
  normal[1] = 0.0;
  normal[2] = 1.0;

  //create origin
  mitk::Point3D origin;
  origin[0] = -27.582859;
  origin[1] = 50;
  origin[2] = 200.27742;

  //initialize plane geometry
  myPlaneGeometry->InitializePlane(origin,normal);

  //output to descripe the test
  std::cout << "Testing PlaneGeometry according to bug #3409" << std::endl;
  std::cout << "Our normal is: " << normal << std::endl;
  std::cout << "So ALL projected points should have exactly the same z-value!" << std::endl;

  //create a number of points
  mitk::Point3D myPoints[5];
  myPoints[0][0] = -27.582859;
  myPoints[0][1] = 50.00;
  myPoints[0][2] = 200.27742;

  myPoints[1][0] = -26.58662;
  myPoints[1][1] = 50.00;
  myPoints[1][2] = 200.19026;

  myPoints[2][0] = -26.58662;
  myPoints[2][1] = 50.00;
  myPoints[2][2] = 200.33124;

  myPoints[3][0] = 104.58662;
  myPoints[3][1] = 452.12313;
  myPoints[3][2] = 866.41236;

  myPoints[4][0] = -207.58662;
  myPoints[4][1] = 312.00;
  myPoints[4][2] = -300.12346;

  //project points onto plane
  mitk::Point3D myProjectedPoints[5];
  for ( unsigned int i = 0; i < 5; ++i )
  {
    myProjectedPoints[i] = myPlaneGeometry->ProjectPointOntoPlane( myPoints[i] );
  }

  //compare z-values with z-value of plane (should be equal)
  bool allPointsOnPlane = true;
  for ( unsigned int i = 0; i < 5; ++i )
  {
    if ( fabs(myProjectedPoints[i][2] - origin[2]) > mitk::sqrteps )
    {
      allPointsOnPlane = false;
    }
  }
  if (!allPointsOnPlane)
    {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout<<"[PASSED]"<<std::endl;
    return EXIT_SUCCESS;
    }
}

mitk::PlaneGeometry::Pointer  createPlaneGeometry()
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

   mitk::PlaneGeometry::Pointer geometry2D = mitk::PlaneGeometry::New();
   geometry2D->SetIndexToWorldTransform(myTransform);
   geometry2D->SetSpacing(mySpacing);
   geometry2D->SetOrigin(myOrigin);
   return geometry2D;
}

int testPlaneGeometryCloning()
{
  mitk::PlaneGeometry::Pointer geometry2D = createPlaneGeometry();

  try
  {
    mitk::PlaneGeometry::Pointer clone = geometry2D->Clone();
    itk::Matrix<mitk::ScalarType,3,3> matrix = clone->GetIndexToWorldTransform()->GetMatrix();
    MITK_TEST_CONDITION(matrix[0][0] == 31, "Test if matrix element exists...");

    double origin = geometry2D->GetOrigin()[0];
    MITK_TEST_CONDITION(mitk::Equal(origin, 8),"First Point of origin as expected...");

    double spacing = geometry2D->GetSpacing()[0];
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

bool compareMatrix(itk::Matrix<mitk::ScalarType, 3,3> left, itk::Matrix<mitk::ScalarType, 3,3> right)
{
  bool equal = true;
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      equal &= mitk::Equal(left[i][j], right[i][j]);
  return equal;
}

int testPlaneGeometryInitializeOrder()
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

   mitk::PlaneGeometry::Pointer geometry2D1 = mitk::PlaneGeometry::New();
   geometry2D1->SetIndexToWorldTransform(myTransform);
   geometry2D1->SetSpacing(mySpacing);
   geometry2D1->SetOrigin(myOrigin);

   mitk::PlaneGeometry::Pointer geometry2D2 = mitk::PlaneGeometry::New();
   geometry2D2->SetSpacing(mySpacing);
   geometry2D2->SetOrigin(myOrigin);
   geometry2D2->SetIndexToWorldTransform(myTransform);

   mitk::PlaneGeometry::Pointer geometry2D3 = mitk::PlaneGeometry::New();
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

int mitkPlaneGeometryTest(int /*argc*/, char* /*argv*/[])
{
  int result;

  /*
  // the following can be used to reproduce a bug in ITK matrix inversion
  // which was found while investigating bug #1210.
  result = TestCase1210();
  if(result!=EXIT_SUCCESS)
    return result;
  */

  mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();

  mitk::Point3D origin;
  mitk::Vector3D right, bottom, normal;
  mitk::ScalarType width, height;
  mitk::ScalarType widthInMM, heightInMM, thicknessInMM;

  width  = 100;    widthInMM  = width;
  height = 200;    heightInMM = height;
  thicknessInMM = 1.0;
  mitk::FillVector3D(origin, 4.5,              7.3, 11.2);
  mitk::FillVector3D(right,  widthInMM,          0, 0);
  mitk::FillVector3D(bottom,         0, heightInMM, 0);
  mitk::FillVector3D(normal,         0,          0, thicknessInMM);

  std::cout << "Testing InitializeStandardPlane(rightVector, downVector, spacing = NULL): "<<std::endl;
  planegeometry->InitializeStandardPlane(right.GetVnlVector(), bottom.GetVnlVector());

  std::cout << "Testing width, height and thickness (in units): ";
  if((mitk::Equal(planegeometry->GetExtent(0),width, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtent(1),height, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtent(2),1, testEps)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm): ";
  if((mitk::Equal(planegeometry->GetExtentInMM(0),widthInMM, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtentInMM(1),heightInMM, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtentInMM(2),thicknessInMM, testEps)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector(): ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), right, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;



  std::cout << "Testing InitializeStandardPlane(rightVector, downVector, spacing = {1.0, 1.0, 1.5}): "<<std::endl;
  mitk::Vector3D spacing;
  thicknessInMM = 1.5;
  normal.Normalize(); normal *= thicknessInMM;
  mitk::FillVector3D(spacing, 1.0, 1.0, thicknessInMM);
  planegeometry->InitializeStandardPlane(right.GetVnlVector(), bottom.GetVnlVector(), &spacing);

  std::cout << "Testing width, height and thickness (in units): ";
  if((mitk::Equal(planegeometry->GetExtent(0),width, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtent(1),height, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtent(2),1, testEps)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm): ";
  if((mitk::Equal(planegeometry->GetExtentInMM(0),widthInMM, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtentInMM(1),heightInMM, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtentInMM(2),thicknessInMM, testEps)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector(): ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), right, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;



  std::cout << "Testing SetExtentInMM(2, ...), querying by GetExtentInMM(2): ";
  thicknessInMM = 3.5;
  normal.Normalize(); normal *= thicknessInMM;
  planegeometry->SetExtentInMM(2, thicknessInMM);
  if(mitk::Equal(planegeometry->GetExtentInMM(2),thicknessInMM, testEps)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing SetExtentInMM(2, ...), querying by GetAxisVector(2) and comparing to normal: ";
  if(mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing SetOrigin: ";
  planegeometry->SetOrigin(origin);
  if(mitk::Equal(planegeometry->GetOrigin(), origin, testEps)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() after SetOrigin: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), right, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  result = mappingTests2D(planegeometry, width, height, widthInMM, heightInMM, origin, right, bottom);
  if(result!=EXIT_SUCCESS)
    return result;



  std::cout << "Changing the IndexToWorldTransform to a rotated version by SetIndexToWorldTransform() (keep origin): "<<std::endl;
  mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
  mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
  vnlmatrix = planegeometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix();
  mitk::VnlVector axis(3);
  mitk::FillVector3D(axis, 1.0, 1.0, 1.0); axis.normalize();
  vnl_quaternion<mitk::ScalarType> rotation(axis, 0.223);
  vnlmatrix = rotation.rotation_matrix_transpose()*vnlmatrix;
  mitk::Matrix3D matrix;
  matrix = vnlmatrix;
  transform->SetMatrix(matrix);
  transform->SetOffset(planegeometry->GetIndexToWorldTransform()->GetOffset());

  right.SetVnlVector( rotation.rotation_matrix_transpose()*right.GetVnlVector() );
  bottom.SetVnlVector(rotation.rotation_matrix_transpose()*bottom.GetVnlVector());
  normal.SetVnlVector(rotation.rotation_matrix_transpose()*normal.GetVnlVector());
  planegeometry->SetIndexToWorldTransform(transform);

  //The origin changed,because m_Origin=m_IndexToWorldTransform->GetOffset()+GetAxisVector(2)*0.5
  //and the AxisVector changes due to the rotation. In other words: the rotation was done around
  //the corner of the box, not around the planes origin. Now change it to a rotation around
  //the origin, simply by re-setting the origin to the original one:
  planegeometry->SetOrigin(origin);
  mitk::Point3D cornerpoint0 = planegeometry->GetCornerPoint(0);

  std::cout << "Testing whether SetIndexToWorldTransform kept origin: ";
  if(mitk::Equal(planegeometry->GetOrigin(), origin, testEps)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  MITK_TEST_OUTPUT( << "Testing consistancy of index and world coordinates. ");
  mitk::Point2D point; point[0] = 4; point[1] = 3;
  mitk::Point2D dummy;
  planegeometry->WorldToIndex(point, dummy);
  planegeometry->IndexToWorld(dummy, dummy);
  MITK_TEST_CONDITION_REQUIRED(dummy == point, "");

  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of rotated version: ";
  if((mitk::Equal(planegeometry->GetExtentInMM(0),widthInMM, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtentInMM(1),heightInMM, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtentInMM(2),thicknessInMM, testEps)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of rotated version: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), right, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector(direction).GetNorm() != planegeometry->GetExtentInMM(direction) of rotated version: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0).GetNorm(),planegeometry->GetExtentInMM(0), testEps)==false) ||
     (mitk::Equal(planegeometry->GetAxisVector(1).GetNorm(),planegeometry->GetExtentInMM(1), testEps)==false) ||
     (mitk::Equal(planegeometry->GetAxisVector(2).GetNorm(),planegeometry->GetExtentInMM(2), testEps)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  result = mappingTests2D(planegeometry, width, height, widthInMM, heightInMM, origin, right, bottom);
  if(result!=EXIT_SUCCESS)
    return result;

  std::cout << "Testing SetSizeInUnits() of rotated version: "<<std::endl;
  width  *= 2;
  height *= 3;
  planegeometry->SetSizeInUnits(width, height);

  std::cout << "Testing width, height and thickness (in units): ";
  if((mitk::Equal(planegeometry->GetExtent(0),width, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtent(1),height, testEps)==false) ||
     (mitk::Equal(planegeometry->GetExtent(2),1, testEps)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of version with changed size in units: ";
  if(!mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(1), heightInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(2), thicknessInMM, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of version with changed size in units: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), right, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector(direction).GetNorm() != planegeometry->GetExtentInMM(direction) of rotated version: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0).GetNorm(),planegeometry->GetExtentInMM(0), testEps)==false) ||
     (mitk::Equal(planegeometry->GetAxisVector(1).GetNorm(),planegeometry->GetExtentInMM(1), testEps)==false) ||
     (mitk::Equal(planegeometry->GetAxisVector(2).GetNorm(),planegeometry->GetExtentInMM(2), testEps)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  result = mappingTests2D(planegeometry, width, height, widthInMM, heightInMM, origin, right, bottom);
  if(result!=EXIT_SUCCESS)
    return result;



  std::cout << "Testing Clone(): ";
  mitk::PlaneGeometry::Pointer clonedplanegeometry = dynamic_cast<mitk::PlaneGeometry*>(planegeometry->Clone().GetPointer());
  if((clonedplanegeometry.IsNull()) || (clonedplanegeometry->GetReferenceCount()!=1))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing origin of cloned version: ";
  if(mitk::Equal(clonedplanegeometry->GetOrigin(), origin, testEps)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in units) of cloned version: ";
  if((mitk::Equal(clonedplanegeometry->GetExtent(0),width, testEps)==false) ||
     (mitk::Equal(clonedplanegeometry->GetExtent(1),height, testEps)==false) ||
     (mitk::Equal(clonedplanegeometry->GetExtent(2),1, testEps)==false)
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of cloned version: ";
  if(!mitk::Equal(clonedplanegeometry->GetExtentInMM(0), widthInMM, testEps) || !mitk::Equal(clonedplanegeometry->GetExtentInMM(1), heightInMM, testEps) || !mitk::Equal(clonedplanegeometry->GetExtentInMM(2), thicknessInMM, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of cloned version: ";
  if((mitk::Equal(clonedplanegeometry->GetAxisVector(0), right, testEps)==false) || (mitk::Equal(clonedplanegeometry->GetAxisVector(1), bottom, testEps)==false) || (mitk::Equal(clonedplanegeometry->GetAxisVector(2), normal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  result = mappingTests2D(clonedplanegeometry, width, height, widthInMM, heightInMM, origin, right, bottom);
  if(result!=EXIT_SUCCESS)
    return result;


  // Clone, move, rotate and test for 'IsParallel' and 'IsOnPlane'
  std::cout << "Testing Clone(): ";
  mitk::PlaneGeometry::Pointer clonedplanegeometry2 = dynamic_cast<mitk::PlaneGeometry*>(planegeometry->Clone().GetPointer());
  if((clonedplanegeometry2.IsNull()) || (clonedplanegeometry2->GetReferenceCount()!=1))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "Testing if cloned and original version are at the same place: ";
  if(mitk::Equal(clonedplanegeometry2->IsOnPlane(planegeometry.GetPointer()), true) ==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing if the origin is on the plane: ";
  if(mitk::Equal(clonedplanegeometry2->IsOnPlane(origin), true)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  mitk::VnlVector newaxis(3);
  mitk::FillVector3D(newaxis, 1.0, 1.0, 1.0); newaxis.normalize();
  vnl_quaternion<mitk::ScalarType> rotation2(newaxis, 0.0);

  mitk::Vector3D clonednormal = clonedplanegeometry2->GetNormal();
  mitk::Point3D clonedorigin = clonedplanegeometry2->GetOrigin();

  mitk::RotationOperation* planerot = new mitk::RotationOperation( mitk::OpROTATE, origin, clonedplanegeometry2->GetAxisVector( 0 ), 180.0 );

  clonedplanegeometry2->ExecuteOperation( planerot );

  std::cout << "Testing whether the flipped plane is still the original plane: ";
  if( mitk::Equal( clonedplanegeometry2->IsOnPlane(planegeometry.GetPointer()), true )==false )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  clonedorigin += clonednormal;
  clonedplanegeometry2->SetOrigin( clonedorigin );

  std::cout << "Testing if the translated (cloned, flipped) plane is parallel to its origin plane: ";
  if( mitk::Equal( clonedplanegeometry2->IsParallel(planegeometry), true )==false )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  delete planerot;

  planerot = new mitk::RotationOperation( mitk::OpROTATE, origin, clonedplanegeometry2->GetAxisVector( 0 ), 0.5 );
  clonedplanegeometry2->ExecuteOperation( planerot );

  std::cout << "Testing if a non-paralell plane gets recognized as not paralell  [rotation +0.5 degree] : ";
  if( mitk::Equal( clonedplanegeometry2->IsParallel(planegeometry), false )==false )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  delete planerot;

  planerot = new mitk::RotationOperation( mitk::OpROTATE, origin, clonedplanegeometry2->GetAxisVector( 0 ), -1.0 );
  clonedplanegeometry2->ExecuteOperation( planerot );

  std::cout << "Testing if a non-paralell plane gets recognized as not paralell  [rotation -0.5 degree] : ";
  if( mitk::Equal( clonedplanegeometry2->IsParallel(planegeometry), false )==false )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  delete planerot;

  planerot = new mitk::RotationOperation( mitk::OpROTATE, origin, clonedplanegeometry2->GetAxisVector( 0 ), 360.5 );
  clonedplanegeometry2->ExecuteOperation( planerot );

  std::cout << "Testing if a non-paralell plane gets recognized as not paralell  [rotation 360 degree] : ";
  if( mitk::Equal( clonedplanegeometry2->IsParallel(planegeometry), true )==false )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  std::cout << "Testing InitializeStandardPlane(clonedplanegeometry, planeorientation = Axial, zPosition = 0, frontside=true): " <<std::endl;
  planegeometry->InitializeStandardPlane(clonedplanegeometry);

  std::cout << "Testing origin of axially initialized version: ";
  if(mitk::Equal(planegeometry->GetOrigin(), origin)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetCornerPoint(0) of axially initialized version: ";
  if(mitk::Equal(planegeometry->GetCornerPoint(0), cornerpoint0)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in units) of axially initialized version (should be same as in mm due to unit spacing, except for thickness, which is always 1): ";
  if(!mitk::Equal(planegeometry->GetExtent(0), width, testEps) || !mitk::Equal(planegeometry->GetExtent(1), height, testEps) || !mitk::Equal(planegeometry->GetExtent(2), 1, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of axially initialized version: ";
  if(!mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(1), heightInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(2), thicknessInMM, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of axially initialized version: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), right, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  result = mappingTests2D(planegeometry, width, height, widthInMM, heightInMM, origin, right, bottom);
  if(result!=EXIT_SUCCESS)
    return result;



  mitk::Vector3D newright, newbottom, newnormal;
  mitk::ScalarType newthicknessInMM;
  std::cout << "Testing InitializeStandardPlane(clonedplanegeometry, planeorientation = Frontal, zPosition = 0, frontside=true): " <<std::endl;
  planegeometry->InitializeStandardPlane(clonedplanegeometry, mitk::PlaneGeometry::Frontal);
  newright = right;
  newbottom = normal; newbottom.Normalize();  newbottom *= thicknessInMM;
  newthicknessInMM = heightInMM/height*1.0/*extent in normal direction is 1*/;
  newnormal = -bottom; newnormal.Normalize(); newnormal *= newthicknessInMM;

  std::cout << "Testing GetCornerPoint(0) of frontally initialized version: ";
  if(mitk::Equal(planegeometry->GetCornerPoint(0), cornerpoint0, testEps)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  //ok, corner was fine, so we can dare to believe the origin is ok.
  origin = planegeometry->GetOrigin();

  std::cout << "Testing width, height and thickness (in units) of frontally initialized version: ";
  if(!mitk::Equal(planegeometry->GetExtent(0), width, testEps) || !mitk::Equal(planegeometry->GetExtent(1), 1, testEps) || !mitk::Equal(planegeometry->GetExtent(2), 1, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of frontally initialized version: ";
  if(!mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(1), thicknessInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(2), newthicknessInMM, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of frontally initialized version: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), newright, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), newbottom, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), newnormal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  result = mappingTests2D(planegeometry, width, 1, widthInMM, thicknessInMM, origin, newright, newbottom);
  if(result!=EXIT_SUCCESS)
    return result;



  std::cout << "Changing plane to in-plane unit spacing using SetSizeInUnits: " <<std::endl;
  planegeometry->SetSizeInUnits(planegeometry->GetExtentInMM(0), planegeometry->GetExtentInMM(1));

  std::cout << "Testing origin of unit spaced, frontally initialized version: ";
  if(mitk::Equal(planegeometry->GetOrigin(), origin, testEps)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in units) of unit spaced, frontally initialized version: ";
  if(!mitk::Equal(planegeometry->GetExtent(0), widthInMM, testEps) || !mitk::Equal(planegeometry->GetExtent(1), thicknessInMM, testEps) || !mitk::Equal(planegeometry->GetExtent(2), 1, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of unit spaced, frontally initialized version: ";
  if(!mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(1), thicknessInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(2), newthicknessInMM, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of unit spaced, frontally initialized version: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), newright, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), newbottom, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), newnormal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  result = mappingTests2D(planegeometry, widthInMM, thicknessInMM, widthInMM, thicknessInMM, origin, newright, newbottom);
  if(result!=EXIT_SUCCESS)
    return result;



  std::cout << "Changing plane to unit spacing also in normal direction using SetExtentInMM(2, 1.0): " <<std::endl;
  planegeometry->SetExtentInMM(2, 1.0);
  newnormal.Normalize();

  std::cout << "Testing origin of unit spaced, frontally initialized version: ";
  if(mitk::Equal(planegeometry->GetOrigin(), origin, testEps)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in units) of unit spaced, frontally initialized version: ";
  if(!mitk::Equal(planegeometry->GetExtent(0), widthInMM, testEps) || !mitk::Equal(planegeometry->GetExtent(1), thicknessInMM, testEps) || !mitk::Equal(planegeometry->GetExtent(2), 1, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of unit spaced, frontally initialized version: ";
  if(!mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(1), thicknessInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(2), 1.0, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of unit spaced, frontally initialized version: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), newright, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), newbottom, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), newnormal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  result = mappingTests2D(planegeometry, widthInMM, thicknessInMM, widthInMM, thicknessInMM, origin, newright, newbottom);
  if(result!=EXIT_SUCCESS)
    return result;



  std::cout << "Testing InitializeStandardPlane(clonedplanegeometry, planeorientation = Sagittal, zPosition = 0, frontside=true): " <<std::endl;
  planegeometry->InitializeStandardPlane(clonedplanegeometry, mitk::PlaneGeometry::Sagittal);
  newright = bottom;
  newthicknessInMM = widthInMM/width*1.0/*extent in normal direction is 1*/;
  newnormal = right; newnormal.Normalize(); newnormal *= newthicknessInMM;

  std::cout << "Testing GetCornerPoint(0) of sagitally initialized version: ";
  if(mitk::Equal(planegeometry->GetCornerPoint(0), cornerpoint0, testEps)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  //ok, corner was fine, so we can dare to believe the origin is ok.
  origin = planegeometry->GetOrigin();

  std::cout << "Testing width, height and thickness (in units) of sagitally initialized version: ";
  if(!mitk::Equal(planegeometry->GetExtent(0), height, testEps) || !mitk::Equal(planegeometry->GetExtent(1), 1, testEps) || !mitk::Equal(planegeometry->GetExtent(2), 1, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of sagitally initialized version: ";
  if(!mitk::Equal(planegeometry->GetExtentInMM(0), heightInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(1), thicknessInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(2), newthicknessInMM, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of sagitally initialized version: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), newright, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), newbottom, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), newnormal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  result = mappingTests2D(planegeometry, height, 1, heightInMM, thicknessInMM, origin, newright, newbottom);
  if(result!=EXIT_SUCCESS)
    return result;



  //set origin back to the one of the axial slice:
  origin = clonedplanegeometry->GetOrigin();
  std::cout << "Testing backside initialization: InitializeStandardPlane(clonedplanegeometry, planeorientation = Axial, zPosition = 0, frontside=false, rotated=true): " <<std::endl;
  planegeometry->InitializeStandardPlane(clonedplanegeometry, mitk::PlaneGeometry::Axial, 0, false, true);
  mitk::Point3D backsideorigin;
  backsideorigin=origin+clonedplanegeometry->GetAxisVector(1);//+clonedplanegeometry->GetAxisVector(2);

  std::cout << "Testing origin of backsidedly, axially initialized version: ";
  if(mitk::Equal(planegeometry->GetOrigin(), backsideorigin, testEps)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetCornerPoint(0) of sagitally initialized version: ";
  mitk::Point3D backsidecornerpoint0;
  backsidecornerpoint0 = cornerpoint0+clonedplanegeometry->GetAxisVector(1);//+clonedplanegeometry->GetAxisVector(2);
  if(mitk::Equal(planegeometry->GetCornerPoint(0), backsidecornerpoint0, testEps)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in units) of backsidedly, axially initialized version (should be same as in mm due to unit spacing, except for thickness, which is always 1): ";
  if(!mitk::Equal(planegeometry->GetExtent(0), width, testEps) || !mitk::Equal(planegeometry->GetExtent(1), height, testEps) || !mitk::Equal(planegeometry->GetExtent(2), 1, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing width, height and thickness (in mm) of backsidedly, axially initialized version: ";
  if(!mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(1), heightInMM, testEps) || !mitk::Equal(planegeometry->GetExtentInMM(2), thicknessInMM, testEps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetAxisVector() of backsidedly, axially initialized version: ";
  if((mitk::Equal(planegeometry->GetAxisVector(0), right, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(1), -bottom, testEps)==false) || (mitk::Equal(planegeometry->GetAxisVector(2), -normal, testEps)==false))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  result = mappingTests2D(planegeometry, width, height, widthInMM, heightInMM, backsideorigin, right, -bottom);
  if(result!=EXIT_SUCCESS)
    return result;


  // test method mitk::PlaneGeometry::ProjectPointOntoPlane()
  // (see also bug #3409)
  result = TestProjectPointOntoPlane();
  if(result!=EXIT_SUCCESS)
    return result;


  // testing mitk::PlaneGeometry::IntersectionPoint()
  std::cout << std::endl;
  std::cout << "Testing IntersectionPoint using given plane and given line:  ";
    result = TestIntersectionPoint();
    if (result != EXIT_SUCCESS) {
        std::cout << "[FAILED]" << std::endl;
        return result;
    }

  std::cout<<"[PASSED]"<<std::endl<<std::endl;

   int result2;

  MITK_TEST_CONDITION_REQUIRED ( (result2 = testPlaneGeometryCloning()) == EXIT_SUCCESS, "");

  // See bug 15990
  // MITK_TEST_CONDITION_REQUIRED ( (result = testPlaneGeometryInitializeOrder()) == EXIT_SUCCESS, "");


  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}

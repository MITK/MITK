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

#include "mitkAffineTransform3D.h"
#include "mitkBaseGeometry.h"
#include "mitkGeometry3D.h"
#include "mitkInteractionConst.h"
#include "mitkLine.h"
#include "mitkPlaneGeometry.h"
#include "mitkRotationOperation.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkThinPlateSplineCurvedGeometry.h"

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.hxx>

#include <fstream>
#include <iomanip>
#include <mitkIOUtil.h>
#include <mitkImage.h>

static const mitk::ScalarType testEps = 1E-9; // the epsilon used in this test == at least float precision.

class mitkPlaneGeometryTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlaneGeometryTestSuite);
  MITK_TEST(TestInitializeStandardPlane);
  MITK_TEST(TestProjectPointOntoPlane);
  MITK_TEST(TestPlaneGeometryCloning);
  MITK_TEST(TestInheritance);
  MITK_TEST(TestSetExtendInMM);
  MITK_TEST(TestRotate);
  MITK_TEST(TestClone);
  MITK_TEST(TestPlaneComparison);
  MITK_TEST(TestAxialInitialization);
  MITK_TEST(TestFrontalInitialization);
  MITK_TEST(TestSaggitalInitialization);
  MITK_TEST(TestLefthandedCoordinateSystem);
  MITK_TEST(TestDominantAxesError);
  MITK_TEST(TestCheckRotationMatrix);

  // Currently commented out, see See bug 15990
  // MITK_TEST(testPlaneGeometryInitializeOrder);
  MITK_TEST(TestIntersectionPoint);
  MITK_TEST(TestCase1210);

  CPPUNIT_TEST_SUITE_END();

private:
  // private test members that are initialized by setUp()
  mitk::PlaneGeometry::Pointer planegeometry;
  mitk::Point3D origin;
  mitk::Vector3D right, bottom, normal, spacing;
  mitk::ScalarType width, height;
  mitk::ScalarType widthInMM, heightInMM, thicknessInMM;

public:
  void setUp() override
  {
    planegeometry = mitk::PlaneGeometry::New();
    width = 100;
    widthInMM = width;
    height = 200;
    heightInMM = height;
    thicknessInMM = 1.0;
    mitk::FillVector3D(origin, 4.5, 7.3, 11.2);
    mitk::FillVector3D(right, widthInMM, 0, 0);
    mitk::FillVector3D(bottom, 0, heightInMM, 0);
    mitk::FillVector3D(normal, 0, 0, thicknessInMM);
    mitk::FillVector3D(spacing, 1.0, 1.0, thicknessInMM);

    planegeometry->InitializeStandardPlane(right, bottom);
    planegeometry->SetOrigin(origin);
    planegeometry->SetSpacing(spacing);
  }

  void tearDown() override {}
  // This test verifies inheritance behaviour, this test will fail if the behaviour changes in the future
  void TestInheritance()
  {
    mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
    mitk::Geometry3D::Pointer g3d = dynamic_cast<mitk::Geometry3D *>(plane.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Planegeometry should not be castable to Geometry 3D", g3d.IsNull());

    mitk::BaseGeometry::Pointer base = dynamic_cast<mitk::BaseGeometry *>(plane.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Planegeometry should be castable to BaseGeometry", base.IsNotNull());

    g3d = mitk::Geometry3D::New();
    base = dynamic_cast<mitk::BaseGeometry *>(g3d.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Geometry3D should be castable to BaseGeometry", base.IsNotNull());

    mitk::SlicedGeometry3D::Pointer sliced = mitk::SlicedGeometry3D::New();
    g3d = dynamic_cast<mitk::Geometry3D *>(sliced.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("SlicedGeometry3D should not be castable to Geometry3D", g3d.IsNull());

    mitk::ThinPlateSplineCurvedGeometry::Pointer thin = mitk::ThinPlateSplineCurvedGeometry::New();
    plane = dynamic_cast<mitk::PlaneGeometry *>(thin.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("AbstractTransformGeometry should be castable to PlaneGeometry", plane.IsNotNull());

    plane = mitk::PlaneGeometry::New();
    mitk::AbstractTransformGeometry::Pointer atg = dynamic_cast<mitk::AbstractTransformGeometry *>(plane.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("PlaneGeometry should not be castable to AbstractTransofrmGeometry", atg.IsNull());
  }

  void TestDominantAxesError()
  {
    auto image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("NotQuiteARotationMatrix.nrrd"));
    auto matrix = image->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().transpose();
    std::vector< int > axes = mitk::PlaneGeometry::CalculateDominantAxes(matrix);
    CPPUNIT_ASSERT_MESSAGE("Domiant axes cannot be determined in this dataset. Output should be default ordering.", axes.at(0)==0 && axes.at(1)==1 && axes.at(2)==2);
  }

  void TestCheckRotationMatrix()
  {
    auto image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("NotQuiteARotationMatrix.nrrd"));
    bool is_rotation = mitk::PlaneGeometry::CheckRotationMatrix(image->GetGeometry()->GetIndexToWorldTransform());
    CPPUNIT_ASSERT_MESSAGE("Since the test data matrix is not quite a rotation matrix, this should be detected.", !is_rotation);
  }

  void TestLefthandedCoordinateSystem()
  {
    /**
     * @brief This method tests InitializeStandardPlane() and IndexToWorld()
     * with a left-handed coordinate orientation or indexToWorldMatrix.
     *
     * Of course this test does not use standard Parameters, which are right-handed.
     * See also discussion of bug #11477: http://bugs.mitk.org/show_bug.cgi?id=11477
     */
    planegeometry = mitk::PlaneGeometry::New();
    width = 100;
    widthInMM = 5;
    height = 200;
    heightInMM = 3;
    thicknessInMM = 1.0;
    mitk::FillVector3D(right, widthInMM, 0, 0);
    mitk::FillVector3D(bottom, 0, heightInMM, 0);
    // This one negative sign results in lefthanded coordinate orientation and det(matrix) < 0.
    mitk::FillVector3D(normal, 0, 0, -thicknessInMM);

    mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();

    mitk::AffineTransform3D::MatrixType matrix;
    mitk::AffineTransform3D::MatrixType::InternalMatrixType &vnl_matrix = matrix.GetVnlMatrix();

    vnl_matrix.set_column(0, right);
    vnl_matrix.set_column(1, bottom);
    vnl_matrix.set_column(2, normal);

    // making sure that we didn't screw up this special test case or else fail deadly:
    assert(vnl_determinant(vnl_matrix) < 0.0);

    transform->SetIdentity();
    transform->SetMatrix(matrix);

    planegeometry->InitializeStandardPlane(width, height, transform); // Crux of the matter.
    CPPUNIT_ASSERT_MESSAGE(
      "Testing if IndexToWorldMatrix is correct after InitializeStandardPlane( width, height, transform ) ",
      mitk::MatrixEqualElementWise(planegeometry->GetIndexToWorldTransform()->GetMatrix(), matrix));

    mitk::Point3D p_index;
    p_index[0] = 10.;
    p_index[1] = 10.;
    p_index[2] = 0.;
    mitk::Point3D p_world;
    mitk::Point3D p_expectedResult;
    p_expectedResult[0] = 50.;
    p_expectedResult[1] = 30.;
    p_expectedResult[2] = 0.;

    ((mitk::BaseGeometry::Pointer)planegeometry)->IndexToWorld(p_index, p_world); // Crux of the matter.
    CPPUNIT_ASSERT_MESSAGE("Testing if IndexToWorld(a,b) function works correctly with lefthanded matrix ",
                           mitk::Equal(p_world, p_expectedResult, testEps));
  }

  // See bug 1210
  // Test does not use standard Parameters
  void TestCase1210()
  {
    mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();

    mitk::Point3D origin;
    mitk::Vector3D right, down, spacing;

    mitk::FillVector3D(origin, 4.5, 7.3, 11.2);
    mitk::FillVector3D(right, 1.015625, 1.015625, 1.1999969482421875);

    mitk::FillVector3D(down, 1.4012984643248170709237295832899161312802619418765e-45, 0, 0);
    mitk::FillVector3D(spacing,
                       0,
                       1.4713633875410579244699160624544119378442750389703e-43,
                       9.2806360452222355258639080851310540729807238879469e-32);

    std::cout << "Testing InitializeStandardPlane(rightVector, downVector, spacing = nullptr): " << std::endl;
    CPPUNIT_ASSERT_NO_THROW(planegeometry->InitializeStandardPlane(right, down, &spacing));
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
   * we test a position where the distance between the correspoinding points is < 0 and another position where the
   * distance is > 0.
   *
   */
  // Test does not use standard Parameters
  void TestIntersectionPoint()
  {
    // init plane with its parameter
    mitk::PlaneGeometry::Pointer myPlaneGeometry = mitk::PlaneGeometry::New();

    mitk::Point3D origin;
    origin[0] = 0.0;
    origin[1] = 2.0;
    origin[2] = 0.0;

    mitk::Vector3D normal;
    normal[0] = 0.0;
    normal[1] = 1.0;
    normal[2] = 0.0;

    myPlaneGeometry->InitializePlane(origin, normal);

    // generate points and line for intersection testing
    // point distance of given line > 1
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

    mitk::Line3D xingline(pointP1, lineDirection);
    mitk::Point3D calcXingPoint;
    myPlaneGeometry->IntersectionPoint(xingline, calcXingPoint);

    // point distance of given line < 1
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

    mitk::Line3D xingline2(pointP3, lineDirection2);
    mitk::Point3D calcXingPoint2;
    myPlaneGeometry->IntersectionPoint(xingline2, calcXingPoint2);

    // intersection points must be the same
    CPPUNIT_ASSERT_MESSAGE("Failed to calculate Intersection Point", calcXingPoint == calcXingPoint2);
  }

  /**
   * @brief This method tests method ProjectPointOntoPlane.
   *
   * See also bug #3409.
   */
  // Test does not use standard Parameters
  void TestProjectPointOntoPlane()
  {
    mitk::PlaneGeometry::Pointer myPlaneGeometry = mitk::PlaneGeometry::New();

    // create normal
    mitk::Vector3D normal;
    normal[0] = 0.0;
    normal[1] = 0.0;
    normal[2] = 1.0;

    // create origin
    mitk::Point3D origin;
    origin[0] = -27.582859;
    origin[1] = 50;
    origin[2] = 200.27742;

    // initialize plane geometry
    myPlaneGeometry->InitializePlane(origin, normal);

    // output to descripe the test
    std::cout << "Testing PlaneGeometry according to bug #3409" << std::endl;
    std::cout << "Our normal is: " << normal << std::endl;
    std::cout << "So ALL projected points should have exactly the same z-value!" << std::endl;

    // create a number of points
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

    // project points onto plane
    mitk::Point3D myProjectedPoints[5];
    for (unsigned int i = 0; i < 5; ++i)
    {
      myProjectedPoints[i] = myPlaneGeometry->ProjectPointOntoPlane(myPoints[i]);
    }

    // compare z-values with z-value of plane (should be equal)
    bool allPointsOnPlane = true;
    for (auto &myProjectedPoint : myProjectedPoints)
    {
      if (fabs(myProjectedPoint[2] - origin[2]) > mitk::sqrteps)
      {
        allPointsOnPlane = false;
      }
    }
    CPPUNIT_ASSERT_MESSAGE("All points lie not on the same plane", allPointsOnPlane);
  }

  void TestPlaneGeometryCloning()
  {
    mitk::PlaneGeometry::Pointer geometry2D = createPlaneGeometry();

    try
    {
      mitk::PlaneGeometry::Pointer clone = geometry2D->Clone();
      itk::Matrix<mitk::ScalarType, 3, 3> matrix = clone->GetIndexToWorldTransform()->GetMatrix();
      CPPUNIT_ASSERT_MESSAGE("Test if matrix element exists...", matrix[0][0] == 31);

      double origin = geometry2D->GetOrigin()[0];
      CPPUNIT_ASSERT_MESSAGE("First Point of origin as expected...", mitk::Equal(origin, 8));

      double spacing = geometry2D->GetSpacing()[0];
      CPPUNIT_ASSERT_MESSAGE("First Point of spacing as expected...", mitk::Equal(spacing, 31));
    }
    catch (...)
    {
      CPPUNIT_FAIL("Error during access on a member of cloned geometry");
    }
    // direction [row] [coloum]
    MITK_TEST_OUTPUT(<< "Casting a rotated 2D ITK Image to a MITK Image and check if Geometry is still same");
  }

  void TestPlaneGeometryInitializeOrder()
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
    itk::Matrix<mitk::ScalarType, 3, 3> transMatrix;
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

    CPPUNIT_ASSERT_MESSAGE("Origin of Geometry 1 matches that of Geometry 2.",
                           mitk::Equal(geometry2D1->GetOrigin(), geometry2D2->GetOrigin()));
    CPPUNIT_ASSERT_MESSAGE("Origin of Geometry 1 match those of Geometry 3.",
                           mitk::Equal(geometry2D1->GetOrigin(), geometry2D3->GetOrigin()));
    CPPUNIT_ASSERT_MESSAGE("Origin of Geometry 2 match those of Geometry 3.",
                           mitk::Equal(geometry2D2->GetOrigin(), geometry2D3->GetOrigin()));

    CPPUNIT_ASSERT_MESSAGE("Spacing of Geometry 1 match those of Geometry 2.",
                           mitk::Equal(geometry2D1->GetSpacing(), geometry2D2->GetSpacing()));
    CPPUNIT_ASSERT_MESSAGE("Spacing of Geometry 1 match those of Geometry 3.",
                           mitk::Equal(geometry2D1->GetSpacing(), geometry2D3->GetSpacing()));
    CPPUNIT_ASSERT_MESSAGE("Spacing of Geometry 2 match those of Geometry 3.",
                           mitk::Equal(geometry2D2->GetSpacing(), geometry2D3->GetSpacing()));

    CPPUNIT_ASSERT_MESSAGE("Transformation of Geometry 1 match those of Geometry 2.",
                           compareMatrix(geometry2D1->GetIndexToWorldTransform()->GetMatrix(),
                                         geometry2D2->GetIndexToWorldTransform()->GetMatrix()));
    CPPUNIT_ASSERT_MESSAGE("Transformation of Geometry 1 match those of Geometry 3.",
                           compareMatrix(geometry2D1->GetIndexToWorldTransform()->GetMatrix(),
                                         geometry2D3->GetIndexToWorldTransform()->GetMatrix()));
    CPPUNIT_ASSERT_MESSAGE("Transformation of Geometry 2 match those of Geometry 3.",
                           compareMatrix(geometry2D2->GetIndexToWorldTransform()->GetMatrix(),
                                         geometry2D3->GetIndexToWorldTransform()->GetMatrix()));
  }

  void TestInitializeStandardPlane()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with default Spacing: width",
                           mitk::Equal(planegeometry->GetExtent(0), width, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with default Spacing: height",
                           mitk::Equal(planegeometry->GetExtent(1), height, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with default Spacing: depth",
                           mitk::Equal(planegeometry->GetExtent(2), 1, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with default Spacing: width in mm",
                           mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with default Spacing: heght in mm",
                           mitk::Equal(planegeometry->GetExtentInMM(1), heightInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with default Spacing: depth in mm",
                           mitk::Equal(planegeometry->GetExtentInMM(2), thicknessInMM, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with default Spacing: AxisVectorRight",
                           mitk::Equal(planegeometry->GetAxisVector(0), right, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with default Spacing: AxisVectorBottom",
                           mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with default Spacing: AxisVectorNormal",
                           mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps));

    mitk::Vector3D spacing;
    thicknessInMM = 1.5;
    normal.Normalize();
    normal *= thicknessInMM;
    mitk::FillVector3D(spacing, 1.0, 1.0, thicknessInMM);
    planegeometry->InitializeStandardPlane(right.GetVnlVector(), bottom.GetVnlVector(), &spacing);

    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with custom Spacing: width",
                           mitk::Equal(planegeometry->GetExtent(0), width, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with custom Spacing: height",
                           mitk::Equal(planegeometry->GetExtent(1), height, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with custom Spacing: depth",
                           mitk::Equal(planegeometry->GetExtent(2), 1, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with custom Spacing: width in mm",
                           mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with custom Spacing: height in mm",
                           mitk::Equal(planegeometry->GetExtentInMM(1), heightInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with custom Spacing: depth in mm",
                           mitk::Equal(planegeometry->GetExtentInMM(2), thicknessInMM, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with custom Spacing: AxisVectorRight",
                           mitk::Equal(planegeometry->GetAxisVector(0), right, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with custom Spacing: AxisVectorBottom",
                           mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing correct Standard Plane initialization with custom Spacing: AxisVectorNormal",
                           mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps));

    ;
  }

  void TestSetExtendInMM()
  {
    normal.Normalize();
    normal *= thicknessInMM;
    planegeometry->SetExtentInMM(2, thicknessInMM);
    CPPUNIT_ASSERT_MESSAGE("Testing SetExtentInMM(2, ...), querying by GetExtentInMM(2): ",
                           mitk::Equal(planegeometry->GetExtentInMM(2), thicknessInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing SetExtentInMM(2, ...), querying by GetAxisVector(2) and comparing to normal: ",
                           mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps));

    planegeometry->SetOrigin(origin);
    CPPUNIT_ASSERT_MESSAGE("Testing SetOrigin", mitk::Equal(planegeometry->GetOrigin(), origin, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() after SetOrigin: Right",
                           mitk::Equal(planegeometry->GetAxisVector(0), right, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() after SetOrigin: Bottom",
                           mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() after SetOrigin: Normal",
                           mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps));

    mappingTests2D(planegeometry, width, height, widthInMM, heightInMM, origin, right, bottom);
  }

  void TestRotate()
  {
    // Changing the IndexToWorldTransform to a rotated version by SetIndexToWorldTransform() (keep origin):
    mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
    mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
    vnlmatrix = planegeometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix();
    mitk::VnlVector axis(3);
    mitk::FillVector3D(axis, 1.0, 1.0, 1.0);
    axis.normalize();
    vnl_quaternion<mitk::ScalarType> rotation(axis, 0.223);
    vnlmatrix = rotation.rotation_matrix_transpose() * vnlmatrix;
    mitk::Matrix3D matrix;
    matrix = vnlmatrix;
    transform->SetMatrix(matrix);
    transform->SetOffset(planegeometry->GetIndexToWorldTransform()->GetOffset());

    right.SetVnlVector(rotation.rotation_matrix_transpose() * right.GetVnlVector());
    bottom.SetVnlVector(rotation.rotation_matrix_transpose() * bottom.GetVnlVector());
    normal.SetVnlVector(rotation.rotation_matrix_transpose() * normal.GetVnlVector());
    planegeometry->SetIndexToWorldTransform(transform);

    // The origin changed,because m_Origin=m_IndexToWorldTransform->GetOffset()+GetAxisVector(2)*0.5
    // and the AxisVector changes due to the rotation. In other words: the rotation was done around
    // the corner of the box, not around the planes origin. Now change it to a rotation around
    // the origin, simply by re-setting the origin to the original one:
    planegeometry->SetOrigin(origin);

    CPPUNIT_ASSERT_MESSAGE("Testing whether SetIndexToWorldTransform kept origin: ",
                           mitk::Equal(planegeometry->GetOrigin(), origin, testEps));

    mitk::Point2D point;
    point[0] = 4;
    point[1] = 3;
    mitk::Point2D dummy;
    planegeometry->WorldToIndex(point, dummy);
    planegeometry->IndexToWorld(dummy, dummy);
    CPPUNIT_ASSERT_MESSAGE("Testing consistency of index and world coordinates.", dummy == point);

    CPPUNIT_ASSERT_MESSAGE("Testing width of rotated version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing height of rotated version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(1), heightInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing thickness of rotated version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(2), thicknessInMM, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of rotated version: right ",
                           mitk::Equal(planegeometry->GetAxisVector(0), right, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of rotated version: bottom",
                           mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of rotated version: normal",
                           mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps));

    CPPUNIT_ASSERT_MESSAGE(
      "Testing GetAxisVector(direction).GetNorm() != planegeometry->GetExtentInMM(direction) of rotated version: ",
      mitk::Equal(planegeometry->GetAxisVector(0).GetNorm(), planegeometry->GetExtentInMM(0), testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing GetAxisVector(direction).GetNorm() != planegeometry->GetExtentInMM(direction) of rotated version: ",
      mitk::Equal(planegeometry->GetAxisVector(1).GetNorm(), planegeometry->GetExtentInMM(1), testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing GetAxisVector(direction).GetNorm() != planegeometry->GetExtentInMM(direction) of rotated version: ",
      mitk::Equal(planegeometry->GetAxisVector(2).GetNorm(), planegeometry->GetExtentInMM(2), testEps));

    mappingTests2D(planegeometry, width, height, widthInMM, heightInMM, origin, right, bottom);

    width *= 2;
    height *= 3;
    planegeometry->SetSizeInUnits(width, height);
    CPPUNIT_ASSERT_MESSAGE("Testing SetSizeInUnits() of rotated version: ",
                           mitk::Equal(planegeometry->GetExtent(0), width, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing SetSizeInUnits() of rotated version: ",
                           mitk::Equal(planegeometry->GetExtent(1), height, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing SetSizeInUnits() of rotated version: ",
                           mitk::Equal(planegeometry->GetExtent(2), 1, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing width (in mm) of version with changed size in units: ",
                           mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing height (in mm) of version with changed size in units: ",
                           mitk::Equal(planegeometry->GetExtentInMM(1), heightInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing thickness (in mm) of version with changed size in units: ",
                           mitk::Equal(planegeometry->GetExtentInMM(2), thicknessInMM, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of version with changed size in units: right ",
                           mitk::Equal(planegeometry->GetAxisVector(0), right, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of version with changed size in units: bottom",
                           mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of version with changed size in units: normal",
                           mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps));

    CPPUNIT_ASSERT_MESSAGE(
      "Testing GetAxisVector(direction).GetNorm() != planegeometry->GetExtentInMM(direction) of rotated version: ",
      mitk::Equal(planegeometry->GetAxisVector(0).GetNorm(), planegeometry->GetExtentInMM(0), testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing GetAxisVector(direction).GetNorm() != planegeometry->GetExtentInMM(direction) of rotated version: ",
      mitk::Equal(planegeometry->GetAxisVector(1).GetNorm(), planegeometry->GetExtentInMM(1), testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing GetAxisVector(direction).GetNorm() != planegeometry->GetExtentInMM(direction) of rotated version: ",
      mitk::Equal(planegeometry->GetAxisVector(2).GetNorm(), planegeometry->GetExtentInMM(2), testEps));

    mappingTests2D(planegeometry, width, height, widthInMM, heightInMM, origin, right, bottom);
  }

  void TestClone()
  {
    mitk::PlaneGeometry::Pointer clonedplanegeometry =
      dynamic_cast<mitk::PlaneGeometry *>(planegeometry->Clone().GetPointer());
    // Cave: Statement below is negated!
    CPPUNIT_ASSERT_MESSAGE("Testing Clone(): ",
                           !((clonedplanegeometry.IsNull()) || (clonedplanegeometry->GetReferenceCount() != 1)));
    CPPUNIT_ASSERT_MESSAGE("Testing origin of cloned version: ",
                           mitk::Equal(clonedplanegeometry->GetOrigin(), origin, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing width (in units) of cloned version: ",
                           mitk::Equal(clonedplanegeometry->GetExtent(0), width, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing height (in units) of cloned version: ",
                           mitk::Equal(clonedplanegeometry->GetExtent(1), height, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing extent (in units) of cloned version: ",
                           mitk::Equal(clonedplanegeometry->GetExtent(2), 1, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing width (in mm) of cloned version: ",
                           mitk::Equal(clonedplanegeometry->GetExtentInMM(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing height (in mm) of cloned version: ",
                           mitk::Equal(clonedplanegeometry->GetExtentInMM(1), heightInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing thickness (in mm) of cloned version: ",
                           mitk::Equal(clonedplanegeometry->GetExtentInMM(2), thicknessInMM, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of cloned version: right",
                           mitk::Equal(clonedplanegeometry->GetAxisVector(0), right, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of cloned version: bottom",
                           mitk::Equal(clonedplanegeometry->GetAxisVector(1), bottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of cloned version: normal",
                           mitk::Equal(clonedplanegeometry->GetAxisVector(2), normal, testEps));

    mappingTests2D(clonedplanegeometry, width, height, widthInMM, heightInMM, origin, right, bottom);
  }

  void TestSaggitalInitialization()
  {
    mitk::Point3D cornerpoint0 = planegeometry->GetCornerPoint(0);
    mitk::PlaneGeometry::Pointer clonedplanegeometry = planegeometry->Clone();

    // Testing InitializeStandardPlane(clonedplanegeometry, planeorientation = Sagittal, zPosition = 0, frontside=true):
    planegeometry->InitializeStandardPlane(clonedplanegeometry, mitk::PlaneGeometry::Sagittal);

    mitk::Vector3D newright, newbottom, newnormal;
    mitk::ScalarType newthicknessInMM;

    newright = bottom;
    newthicknessInMM = widthInMM / width * 1.0; // extent in normal direction is 1;
    newnormal = right;
    newnormal.Normalize();
    newnormal *= newthicknessInMM;
    newbottom = normal;
    newbottom.Normalize();
    newbottom *= thicknessInMM;

    CPPUNIT_ASSERT_MESSAGE("Testing GetCornerPoint(0) of sagitally initialized version:",
                           mitk::Equal(planegeometry->GetCornerPoint(0), cornerpoint0, testEps));

    // ok, corner was fine, so we can dare to believe the origin is ok.
    origin = planegeometry->GetOrigin();

    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in units) of sagitally initialized version: ",
                           mitk::Equal(planegeometry->GetExtent(0), height, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in units) of sagitally initialized version: ",
                           mitk::Equal(planegeometry->GetExtent(1), 1, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in units) of sagitally initialized version: ",
                           mitk::Equal(planegeometry->GetExtent(2), 1, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in mm) of sagitally initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(0), heightInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in mm) of sagitally initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(1), thicknessInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in mm) of sagitally initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(2), newthicknessInMM, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of sagitally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(0), newright, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of sagitally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(1), newbottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of sagitally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(2), newnormal, testEps));

    mappingTests2D(planegeometry, height, 1, heightInMM, thicknessInMM, origin, newright, newbottom);

    // set origin back to the one of the axial slice:
    origin = clonedplanegeometry->GetOrigin();
    // Testing backside initialization: InitializeStandardPlane(clonedplanegeometry, planeorientation = Axial, zPosition
    // = 0, frontside=false, rotated=true):
    planegeometry->InitializeStandardPlane(clonedplanegeometry, mitk::PlaneGeometry::Axial, 0, false, true);
    mitk::Point3D backsideorigin;
    backsideorigin = origin + clonedplanegeometry->GetAxisVector(1); //+clonedplanegeometry->GetAxisVector(2);

    CPPUNIT_ASSERT_MESSAGE("Testing origin of backsidedly, axially initialized version: ",
                           mitk::Equal(planegeometry->GetOrigin(), backsideorigin, testEps));

    mitk::Point3D backsidecornerpoint0;
    backsidecornerpoint0 =
      cornerpoint0 + clonedplanegeometry->GetAxisVector(1); //+clonedplanegeometry->GetAxisVector(2);
    CPPUNIT_ASSERT_MESSAGE("Testing GetCornerPoint(0) of sagitally initialized version: ",
                           mitk::Equal(planegeometry->GetCornerPoint(0), backsidecornerpoint0, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in units) of backsidedly, axially initialized version "
                           "(should be same as in mm due to unit spacing, except for thickness, which is always 1): ",
                           mitk::Equal(planegeometry->GetExtent(0), width, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in units) of backsidedly, axially initialized version "
                           "(should be same as in mm due to unit spacing, except for thickness, which is always 1): ",
                           mitk::Equal(planegeometry->GetExtent(1), height, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in units) of backsidedly, axially initialized version "
                           "(should be same as in mm due to unit spacing, except for thickness, which is always 1): ",
                           mitk::Equal(planegeometry->GetExtent(2), 1, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in mm) of backsidedly, axially initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in mm) of backsidedly, axially initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(1), heightInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing width, height and thickness (in mm) of backsidedly, axially initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(2), thicknessInMM, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of backsidedly, axially initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(0), right, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of backsidedly, axially initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(1), -bottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of backsidedly, axially initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps)); // T22254: Flipped sign

    mappingTests2D(planegeometry, width, height, widthInMM, heightInMM, backsideorigin, right, -bottom);
  }

  void TestFrontalInitialization()
  {
    mitk::Point3D cornerpoint0 = planegeometry->GetCornerPoint(0);

    mitk::PlaneGeometry::Pointer clonedplanegeometry =
      dynamic_cast<mitk::PlaneGeometry *>(planegeometry->Clone().GetPointer());
    //--------

    mitk::Vector3D newright, newbottom, newnormal;
    mitk::ScalarType newthicknessInMM;

    // Testing InitializeStandardPlane(clonedplanegeometry, planeorientation = Frontal, zPosition = 0, frontside=true)
    planegeometry->InitializeStandardPlane(clonedplanegeometry, mitk::PlaneGeometry::Frontal);
    newright = right;
    newbottom = normal;
    newbottom.Normalize();
    newbottom *= thicknessInMM;
    newthicknessInMM = heightInMM / height * 1.0 /*extent in normal direction is 1*/;
    newnormal = -bottom;
    newnormal.Normalize();
    newnormal *= newthicknessInMM;

    CPPUNIT_ASSERT_MESSAGE("Testing GetCornerPoint(0) of frontally initialized version: ",
                           mitk::Equal(planegeometry->GetCornerPoint(0), cornerpoint0, testEps));

    // ok, corner was fine, so we can dare to believe the origin is ok.
    origin = planegeometry->GetOrigin();

    CPPUNIT_ASSERT_MESSAGE("Testing width (in units) of frontally initialized version: ",
                           mitk::Equal(planegeometry->GetExtent(0), width, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing height (in units) of frontally initialized version: ",
                           mitk::Equal(planegeometry->GetExtent(1), 1, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing thickness (in units) of frontally initialized version: ",
                           mitk::Equal(planegeometry->GetExtent(2), 1, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing width (in mm) of frontally initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing height (in mm) of frontally initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(1), thicknessInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing thickness (in mm) of frontally initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(2), newthicknessInMM, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of frontally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(0), newright, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of frontally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(1), newbottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of frontally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(2), -newnormal, testEps)); // T22254: Flipped sign

    mappingTests2D(planegeometry, width, 1, widthInMM, thicknessInMM, origin, newright, newbottom);

    // Changing plane to in-plane unit spacing using SetSizeInUnits:
    planegeometry->SetSizeInUnits(planegeometry->GetExtentInMM(0), planegeometry->GetExtentInMM(1));

    CPPUNIT_ASSERT_MESSAGE("Testing origin of unit spaced, frontally initialized version: ",
                           mitk::Equal(planegeometry->GetOrigin(), origin, testEps));

    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in units) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtent(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in units) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtent(1), thicknessInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in units) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtent(2), 1, testEps));

    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in mm) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in mm) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtentInMM(1), thicknessInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in mm) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtentInMM(2), newthicknessInMM, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of unit spaced, frontally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(0), newright, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of unit spaced, frontally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(1), newbottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of unit spaced, frontally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(2), -newnormal, testEps)); // T22254: Flipped sign

    mappingTests2D(planegeometry, widthInMM, thicknessInMM, widthInMM, thicknessInMM, origin, newright, newbottom);

    // Changing plane to unit spacing also in normal direction using SetExtentInMM(2, 1.0):
    planegeometry->SetExtentInMM(2, 1.0);
    newnormal.Normalize();

    CPPUNIT_ASSERT_MESSAGE("Testing origin of unit spaced, frontally initialized version: ",
                           mitk::Equal(planegeometry->GetOrigin(), origin, testEps));

    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in units) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtent(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in units) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtent(1), thicknessInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in units) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtent(2), 1, testEps));

    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in mm) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in mm) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtentInMM(1), thicknessInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE(
      "Testing width, height and thickness (in mm) of unit spaced, frontally initialized version: ",
      mitk::Equal(planegeometry->GetExtentInMM(2), 1.0, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of unit spaced, frontally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(0), newright, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of unit spaced, frontally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(1), newbottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of unit spaced, frontally initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(2), -newnormal, testEps)); // T22254: Flipped sign
    mappingTests2D(planegeometry, widthInMM, thicknessInMM, widthInMM, thicknessInMM, origin, newright, newbottom);
  }

  void TestAxialInitialization()
  {
    mitk::Point3D cornerpoint0 = planegeometry->GetCornerPoint(0);

    // Clone, move, rotate and test for 'IsParallel' and 'IsOnPlane'
    mitk::PlaneGeometry::Pointer clonedplanegeometry =
      dynamic_cast<mitk::PlaneGeometry *>(planegeometry->Clone().GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Testing Clone(): ",
                           !((clonedplanegeometry.IsNull()) || (clonedplanegeometry->GetReferenceCount() != 1)));

    std::cout << "Testing InitializeStandardPlane(clonedplanegeometry, planeorientation = Axial, zPosition = 0, "
                 "frontside=true): "
              << std::endl;
    planegeometry->InitializeStandardPlane(clonedplanegeometry);

    CPPUNIT_ASSERT_MESSAGE("Testing origin of axially initialized version: ",
                           mitk::Equal(planegeometry->GetOrigin(), origin));

    CPPUNIT_ASSERT_MESSAGE("Testing GetCornerPoint(0) of axially initialized version: ",
                           mitk::Equal(planegeometry->GetCornerPoint(0), cornerpoint0));

    CPPUNIT_ASSERT_MESSAGE("Testing width (in units) of axially initialized version (should be same as in mm due to "
                           "unit spacing, except for thickness, which is always 1): ",
                           mitk::Equal(planegeometry->GetExtent(0), width, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing height (in units) of axially initialized version (should be same as in mm due to "
                           "unit spacing, except for thickness, which is always 1): ",
                           mitk::Equal(planegeometry->GetExtent(1), height, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing thickness (in units) of axially initialized version (should be same as in mm due "
                           "to unit spacing, except for thickness, which is always 1): ",
                           mitk::Equal(planegeometry->GetExtent(2), 1, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing width (in mm) of axially initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(0), widthInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing height  (in mm) of axially initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(1), heightInMM, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing thickness (in mm) of axially initialized version: ",
                           mitk::Equal(planegeometry->GetExtentInMM(2), thicknessInMM, testEps));

    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of axially initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(0), right, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of axially initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(1), bottom, testEps));
    CPPUNIT_ASSERT_MESSAGE("Testing GetAxisVector() of axially initialized version: ",
                           mitk::Equal(planegeometry->GetAxisVector(2), normal, testEps));

    mappingTests2D(planegeometry, width, height, widthInMM, heightInMM, origin, right, bottom);
  }

  void TestPlaneComparison()
  {
    // Clone, move, rotate and test for 'IsParallel' and 'IsOnPlane'
    mitk::PlaneGeometry::Pointer clonedplanegeometry2 =
      dynamic_cast<mitk::PlaneGeometry *>(planegeometry->Clone().GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Testing Clone(): ",
                           !((clonedplanegeometry2.IsNull()) || (clonedplanegeometry2->GetReferenceCount() != 1)));
    CPPUNIT_ASSERT_MESSAGE("Testing wheter original and clone are at the same position",
                           clonedplanegeometry2->IsOnPlane(planegeometry.GetPointer()));
    CPPUNIT_ASSERT_MESSAGE(" Asserting that origin is on the plane cloned plane:",
                           clonedplanegeometry2->IsOnPlane(origin));

    mitk::VnlVector newaxis(3);
    mitk::FillVector3D(newaxis, 1.0, 1.0, 1.0);
    newaxis.normalize();
    vnl_quaternion<mitk::ScalarType> rotation2(newaxis, 0.0);

    mitk::Vector3D clonednormal = clonedplanegeometry2->GetNormal();
    mitk::Point3D clonedorigin = clonedplanegeometry2->GetOrigin();

    auto planerot = new mitk::RotationOperation(mitk::OpROTATE, origin, clonedplanegeometry2->GetAxisVector(0), 180.0);

    clonedplanegeometry2->ExecuteOperation(planerot);
    CPPUNIT_ASSERT_MESSAGE(" Asserting that a flipped plane is still on the original plane: ",
                           clonedplanegeometry2->IsOnPlane(planegeometry.GetPointer()));

    clonedorigin += clonednormal;
    clonedplanegeometry2->SetOrigin(clonedorigin);

    CPPUNIT_ASSERT_MESSAGE("Testing if the translated (cloned, flipped) plane is parallel to its origin plane: ",
                           clonedplanegeometry2->IsParallel(planegeometry));
    delete planerot;

    planerot = new mitk::RotationOperation(mitk::OpROTATE, origin, clonedplanegeometry2->GetAxisVector(0), 0.5);
    clonedplanegeometry2->ExecuteOperation(planerot);

    CPPUNIT_ASSERT_MESSAGE("Testing if a non-paralell plane gets recognized as not paralell  [rotation +0.5 degree] : ",
                           !clonedplanegeometry2->IsParallel(planegeometry));
    delete planerot;

    planerot = new mitk::RotationOperation(mitk::OpROTATE, origin, clonedplanegeometry2->GetAxisVector(0), -1.0);
    clonedplanegeometry2->ExecuteOperation(planerot);

    CPPUNIT_ASSERT_MESSAGE("Testing if a non-paralell plane gets recognized as not paralell  [rotation -0.5 degree] : ",
                           !clonedplanegeometry2->IsParallel(planegeometry));
    delete planerot;

    planerot = new mitk::RotationOperation(mitk::OpROTATE, origin, clonedplanegeometry2->GetAxisVector(0), 360.5);
    clonedplanegeometry2->ExecuteOperation(planerot);

    CPPUNIT_ASSERT_MESSAGE("Testing if a non-paralell plane gets recognized as paralell  [rotation 360 degree] : ",
                           clonedplanegeometry2->IsParallel(planegeometry));
  }

private:
  // helper Methods for the Tests

  mitk::PlaneGeometry::Pointer createPlaneGeometry()
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
    itk::Matrix<mitk::ScalarType, 3, 3> transMatrix;
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

  bool compareMatrix(itk::Matrix<mitk::ScalarType, 3, 3> left, itk::Matrix<mitk::ScalarType, 3, 3> right)
  {
    bool equal = true;
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        equal &= mitk::Equal(left[i][j], right[i][j]);
    return equal;
  }

  /**
   * This function tests for correct mapping and is called several times from other tests
   **/
  void mappingTests2D(const mitk::PlaneGeometry *planegeometry,
                      const mitk::ScalarType &width,
                      const mitk::ScalarType &height,
                      const mitk::ScalarType &widthInMM,
                      const mitk::ScalarType &heightInMM,
                      const mitk::Point3D &origin,
                      const mitk::Vector3D &right,
                      const mitk::Vector3D &bottom)
  {
    std::cout << "Testing mapping Map(pt2d_mm(x=widthInMM/2.3,y=heightInMM/2.5), pt3d_mm) and compare with expected: ";
    mitk::Point2D pt2d_mm;
    mitk::Point3D pt3d_mm, expected_pt3d_mm;
    pt2d_mm[0] = widthInMM / 2.3;
    pt2d_mm[1] = heightInMM / 2.5;
    expected_pt3d_mm = origin + right * (pt2d_mm[0] / right.GetNorm()) + bottom * (pt2d_mm[1] / bottom.GetNorm());
    planegeometry->Map(pt2d_mm, pt3d_mm);
    CPPUNIT_ASSERT_MESSAGE(
      "Testing mapping Map(pt2d_mm(x=widthInMM/2.3,y=heightInMM/2.5), pt3d_mm) and compare with expected",
      mitk::Equal(pt3d_mm, expected_pt3d_mm, testEps));

    std::cout << "Testing mapping Map(pt3d_mm, pt2d_mm) and compare with expected: ";
    mitk::Point2D testpt2d_mm;
    planegeometry->Map(pt3d_mm, testpt2d_mm);
    std::cout << std::setprecision(12) << "Expected pt2d_mm " << pt2d_mm << std::endl;
    std::cout << std::setprecision(12) << "Result testpt2d_mm " << testpt2d_mm << std::endl;
    std::cout << std::setprecision(12) << "10*mitk::eps " << 10 * mitk::eps << std::endl;
    // This eps is temporarily set to 10*mitk::eps. See bug #15037 for details.
    CPPUNIT_ASSERT_MESSAGE("Testing mapping Map(pt3d_mm, pt2d_mm) and compare with expected",
                           mitk::Equal(pt2d_mm, testpt2d_mm, 10 * mitk::eps));

    std::cout << "Testing IndexToWorld(pt2d_units, pt2d_mm) and compare with expected: ";
    mitk::Point2D pt2d_units;
    pt2d_units[0] = width / 2.0;
    pt2d_units[1] = height / 2.0;
    pt2d_mm[0] = widthInMM / 2.0;
    pt2d_mm[1] = heightInMM / 2.0;
    planegeometry->IndexToWorld(pt2d_units, testpt2d_mm);
    std::cout << std::setprecision(12) << "Expected pt2d_mm " << pt2d_mm << std::endl;
    std::cout << std::setprecision(12) << "Result testpt2d_mm " << testpt2d_mm << std::endl;
    std::cout << std::setprecision(12) << "10*mitk::eps " << 10 * mitk::eps << std::endl;
    // This eps is temporarily set to 10*mitk::eps. See bug #15037 for details.
    CPPUNIT_ASSERT_MESSAGE("Testing IndexToWorld(pt2d_units, pt2d_mm) and compare with expected: ",
                           mitk::Equal(pt2d_mm, testpt2d_mm, 10 * mitk::eps));

    std::cout << "Testing WorldToIndex(pt2d_mm, pt2d_units) and compare with expected: ";
    mitk::Point2D testpt2d_units;
    planegeometry->WorldToIndex(pt2d_mm, testpt2d_units);

    std::cout << std::setprecision(12) << "Expected pt2d_units " << pt2d_units << std::endl;
    std::cout << std::setprecision(12) << "Result testpt2d_units " << testpt2d_units << std::endl;
    std::cout << std::setprecision(12) << "10*mitk::eps " << 10 * mitk::eps << std::endl;
    // This eps is temporarily set to 10*mitk::eps. See bug #15037 for details.
    CPPUNIT_ASSERT_MESSAGE("Testing WorldToIndex(pt2d_mm, pt2d_units) and compare with expected:",
                           mitk::Equal(pt2d_units, testpt2d_units, 10 * mitk::eps));
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkPlaneGeometry)

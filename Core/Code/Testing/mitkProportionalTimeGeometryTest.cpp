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

#include "mitkProportionalTimeGeometry.h"
#include "mitkGeometry3D.h"
#include "mitkThinPlateSplineCurvedGeometry.h"
#include "mitkSlicedGeometry3D.h"

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>


class mitkProportionalTimeGeometryTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkProportionalTimeGeometryTestSuite);
  MITK_TEST(TestInheritance);
  MITK_TEST(TestProportionalTimeGeometryCloning);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp()
  {
  }

  void tearDown()
  {
  }

  // This test is supposed to verify inheritance behaviour, this test will fail if the behaviour changes in the future
  void TestInheritance()
  {
    mitk::ProportionalTimeGeometry::Pointer ptGeom = mitk::ProportionalTimeGeometry::New();
    mitk::Geometry3D::Pointer g3d = dynamic_cast < mitk::Geometry3D* > ( ptGeom.GetPointer() );
    CPPUNIT_ASSERT_MESSAGE("ProportionalTimeGeometry should not be castable to Geometry3D", g3d.IsNull());

    mitk::TimeGeometry::Pointer base = dynamic_cast < mitk::TimeGeometry* > ( ptGeom.GetPointer() );
    CPPUNIT_ASSERT_MESSAGE("ProportionalTimeGeometry should be castable to TimeGeometry", base.IsNotNull());
  }


  void TestProportionalTimeGeometryCloning()
  {
    mitk::ProportionalTimeGeometry::Pointer geom = CreateProportionalTimeGeometry();

    mitk::ProportionalTimeGeometry::Pointer clone = geom->Clone();
    CPPUNIT_ASSERT_MESSAGE("First time point of clone matches original",
                           clone->GetFirstTimePoint() == 1.1);
    CPPUNIT_ASSERT_MESSAGE("Step duration of clone matches original",
                           clone->GetStepDuration() == 2.2);

    mitk::PlaneGeometry *planeGeom = dynamic_cast<mitk::PlaneGeometry*>(clone->GetGeometryForTimeStep(0).GetPointer());
    itk::Matrix<mitk::ScalarType,3,3> matrix = planeGeom->GetIndexToWorldTransform()->GetMatrix();
    CPPUNIT_ASSERT_MESSAGE("Matrix element [0][0] of clone matches original", matrix[0][0] == 31);

    double origin = planeGeom->GetOrigin()[0];
    CPPUNIT_ASSERT_MESSAGE("First Point of origin of clone matches original", mitk::Equal(origin, 8));

    double spacing = planeGeom->GetSpacing()[0];
    CPPUNIT_ASSERT_MESSAGE("First Point of spacing of clone matches original", mitk::Equal(spacing, 31));
  }

private:
  // helper Methods for the Tests

  mitk::ProportionalTimeGeometry::Pointer  CreateProportionalTimeGeometry()
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

    mitk::PlaneGeometry::Pointer geom2D = mitk::PlaneGeometry::New();
    geom2D->SetIndexToWorldTransform(myTransform);
    geom2D->SetSpacing(mySpacing);
    geom2D->SetOrigin(myOrigin);

    mitk::ProportionalTimeGeometry::Pointer geom = mitk::ProportionalTimeGeometry::New();
    geom->SetFirstTimePoint(1.1);
    geom->SetStepDuration(2.2);
    geom->SetTimeStepGeometry(geom2D, 0);

    return geom;
  }

};
MITK_TEST_SUITE_REGISTRATION(mitkProportionalTimeGeometry)

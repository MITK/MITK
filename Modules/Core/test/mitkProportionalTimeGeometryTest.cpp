/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGeometry3D.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkThinPlateSplineCurvedGeometry.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkProportionalTimeGeometryTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkProportionalTimeGeometryTestSuite);
  MITK_TEST(TestInheritance);
  MITK_TEST(TestProportionalTimeGeometryCloning);
  MITK_TEST(Expand_SingleTimeStepValidAtEveryTimePoint_ConsecutiveTimeSteps);
  MITK_TEST(IsValidAtEveryTimePoint_OnlySingleTimeStepWithoutTime);
  MITK_TEST(TimePointToClampedTimeStep_BeforeFirst_IsFirstStep);
  MITK_TEST(TimePointToClampedTimeStep_Inside_IsContainingStep);
  MITK_TEST(TimePointToClampedTimeStep_StepStart_IsThatStep);
  MITK_TEST(TimePointToClampedTimeStep_PastLast_IsLastStep);
  MITK_TEST(TimePointToClampedTimeStep_NoTimeSteps_Throws);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override {}
  void tearDown() override {}
  // This test is supposed to verify inheritance behaviour, this test will fail if the behaviour changes in the future
  void TestInheritance()
  {
    mitk::ProportionalTimeGeometry::Pointer ptGeom = mitk::ProportionalTimeGeometry::New();
    mitk::Geometry3D::Pointer g3d = dynamic_cast<mitk::Geometry3D *>(ptGeom.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("ProportionalTimeGeometry should not be castable to Geometry3D", g3d.IsNull());

    mitk::TimeGeometry::Pointer base = dynamic_cast<mitk::TimeGeometry *>(ptGeom.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("ProportionalTimeGeometry should be castable to TimeGeometry", base.IsNotNull());
  }

  void TestProportionalTimeGeometryCloning()
  {
    mitk::ProportionalTimeGeometry::Pointer geom = CreateProportionalTimeGeometry();

    mitk::ProportionalTimeGeometry::Pointer clone = geom->Clone();
    CPPUNIT_ASSERT_MESSAGE("First time point of clone matches original", clone->GetFirstTimePoint() == 1.1);
    CPPUNIT_ASSERT_MESSAGE("Step duration of clone matches original", clone->GetStepDuration() == 2.2);

    mitk::PlaneGeometry *planeGeom = dynamic_cast<mitk::PlaneGeometry *>(clone->GetGeometryForTimeStep(0).GetPointer());
    itk::Matrix<mitk::ScalarType, 3, 3> matrix = planeGeom->GetIndexToWorldTransform()->GetMatrix();
    CPPUNIT_ASSERT_MESSAGE("Matrix element [0][0] of clone matches original", matrix[0][0] == 31);

    double origin = planeGeom->GetOrigin()[0];
    CPPUNIT_ASSERT_MESSAGE("First Point of origin of clone matches original", mitk::Equal(origin, 8));

    double spacing = planeGeom->GetSpacing()[0];
    CPPUNIT_ASSERT_MESSAGE("First Point of spacing of clone matches original", mitk::Equal(spacing, 31));
  }

  void Expand_SingleTimeStepValidAtEveryTimePoint_ConsecutiveTimeSteps()
  {
    auto geometry = mitk::ProportionalTimeGeometry::New();
    geometry->Initialize(mitk::Geometry3D::New(), 1);
    CPPUNIT_ASSERT(geometry->IsValidTimePoint(1e6));

    geometry->Expand(3);

    // The same time steps as initializing with three of them.
    auto expected = mitk::ProportionalTimeGeometry::New();
    expected->Initialize(mitk::Geometry3D::New(), 3);

    CPPUNIT_ASSERT_EQUAL(expected->GetFirstTimePoint(), geometry->GetFirstTimePoint());
    CPPUNIT_ASSERT_EQUAL(expected->GetStepDuration(), geometry->GetStepDuration());
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(2), geometry->TimePointToTimeStep(2.5));
  }

  void IsValidAtEveryTimePoint_OnlySingleTimeStepWithoutTime()
  {
    auto geometry = mitk::ProportionalTimeGeometry::New();
    geometry->Initialize(mitk::Geometry3D::New(), 1);
    CPPUNIT_ASSERT(geometry->IsValidAtEveryTimePoint());

    auto timedGeometry = geometry->Clone();
    timedGeometry->SetFirstTimePoint(100.0);
    timedGeometry->SetStepDuration(50.0);
    CPPUNIT_ASSERT(!timedGeometry->IsValidAtEveryTimePoint());

    geometry->Expand(2);
    CPPUNIT_ASSERT(!geometry->IsValidAtEveryTimePoint());
  }

  void TimePointToClampedTimeStep_BeforeFirst_IsFirstStep()
  {
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(0), CreateThreeTimeStepsFrom10()->TimePointToClampedTimeStep(5.0));
  }

  void TimePointToClampedTimeStep_Inside_IsContainingStep()
  {
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(1), CreateThreeTimeStepsFrom10()->TimePointToClampedTimeStep(17.0));
  }

  void TimePointToClampedTimeStep_StepStart_IsThatStep()
  {
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(2), CreateThreeTimeStepsFrom10()->TimePointToClampedTimeStep(20.0));
  }

  void TimePointToClampedTimeStep_PastLast_IsLastStep()
  {
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(2), CreateThreeTimeStepsFrom10()->TimePointToClampedTimeStep(100.0));
  }

  void TimePointToClampedTimeStep_NoTimeSteps_Throws()
  {
    auto geometry = mitk::ProportionalTimeGeometry::New();
    CPPUNIT_ASSERT_THROW(geometry->TimePointToClampedTimeStep(0.0), mitk::NoTimeStepsException);
  }

private:
  // helper Methods for the Tests

  /** Time steps [10, 15), [15, 20) and [20, 25). */
  static mitk::ProportionalTimeGeometry::Pointer CreateThreeTimeStepsFrom10()
  {
    auto geometry = mitk::ProportionalTimeGeometry::New();
    geometry->Initialize(mitk::Geometry3D::New(), 3);
    geometry->SetFirstTimePoint(10.0);
    geometry->SetStepDuration(5.0);
    return geometry;
  }

  mitk::ProportionalTimeGeometry::Pointer CreateProportionalTimeGeometry()
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

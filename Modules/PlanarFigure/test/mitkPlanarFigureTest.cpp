/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarLine.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlaneGeometry.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <functional>

namespace
{
  mitk::Point2D P(double x, double y)
  {
    mitk::Point2D point;
    point[0] = x;
    point[1] = y;
    return point;
  }

  mitk::PlaneGeometry::Pointer CreatePlane()
  {
    mitk::Vector3D spacing;
    spacing.Fill(1.0);

    auto plane = mitk::PlaneGeometry::New();
    plane->InitializeStandardPlane(100.0, 100.0, spacing);
    return plane;
  }

  mitk::PlanarPolygon::Pointer CreateSquare()
  {
    auto polygon = mitk::PlanarPolygon::New();
    polygon->SetPlaneGeometry(CreatePlane());
    polygon->SetControlPoints({ P(10, 10), P(20, 10), P(20, 20), P(10, 20) });
    return polygon;
  }

  bool IsModifiedBy(mitk::PlanarFigure *figure, const std::function<void()> &mutation)
  {
    const auto before = figure->GetMTime();
    mutation();
    return figure->GetMTime() > before;
  }
}

class mitkPlanarFigureTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureTestSuite);

  MITK_TEST(NewFigure_HasNoControlPoints);
  MITK_TEST(Placement_MarksFigureIncomplete);
  MITK_TEST(SetControlPoints_ChecksNumber);
  MITK_TEST(SetControlPoints_WhileBeingPlaced_AcceptsFewerPoints);
  MITK_TEST(InsertControlPoint_OnlyWhereAllowed);
  MITK_TEST(InsertControlPoint_ChecksIndexAndMaximum);
  MITK_TEST(RemoveControlPoint_WhileBeingPlaced_GoesBelowMinimum);
  MITK_TEST(Translate_MovesEveryControlPoint);
  MITK_TEST(MoveHandle_OutOfRange_Throws);
  MITK_TEST(Mutators_ModifyFigure);
  MITK_TEST(Clone_CopiesStateIndependently);

  CPPUNIT_TEST_SUITE_END();

public:
  void NewFigure_HasNoControlPoints()
  {
    auto line = mitk::PlanarLine::New();

    CPPUNIT_ASSERT_EQUAL(0u, line->GetNumberOfControlPoints());
    CPPUNIT_ASSERT(line->GetControlPoints().empty());
    CPPUNIT_ASSERT(line->GetContours().empty());
    CPPUNIT_ASSERT(line->GetMeasurements().empty());
    CPPUNIT_ASSERT(!line->IsComplete());
  }

  void Placement_MarksFigureIncomplete()
  {
    auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(CreatePlane());
    line->BeginPlacement();
    line->SetFromSpan(P(10, 10), P(20, 10));

    CPPUNIT_ASSERT(line->IsBeingPlaced());
    CPPUNIT_ASSERT(!line->IsComplete());

    line->EndPlacement();

    CPPUNIT_ASSERT(!line->IsBeingPlaced());
    CPPUNIT_ASSERT(line->IsComplete());
  }

  void SetControlPoints_ChecksNumber()
  {
    auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(CreatePlane());

    CPPUNIT_ASSERT_THROW(line->SetControlPoints({ P(10, 10) }), mitk::Exception);
    CPPUNIT_ASSERT_THROW(line->SetControlPoints({ P(10, 10), P(20, 10), P(30, 10) }), mitk::Exception);

    line->SetControlPoints({ P(10, 10), P(20, 10) });
    CPPUNIT_ASSERT_EQUAL(2u, line->GetNumberOfControlPoints());
    CPPUNIT_ASSERT(line->IsComplete());
  }

  void SetControlPoints_WhileBeingPlaced_AcceptsFewerPoints()
  {
    auto polygon = mitk::PlanarPolygon::New();
    polygon->SetPlaneGeometry(CreatePlane());
    polygon->BeginPlacement();
    polygon->SetControlPoints({ P(10, 10) });

    CPPUNIT_ASSERT_EQUAL(1u, polygon->GetNumberOfControlPoints());
  }

  void InsertControlPoint_OnlyWhereAllowed()
  {
    auto polygon = CreateSquare();
    polygon->InsertControlPoint(1, P(15, 5));

    CPPUNIT_ASSERT_EQUAL(5u, polygon->GetNumberOfControlPoints());
    CPPUNIT_ASSERT(polygon->GetControlPoint(1) == P(15, 5));

    auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(CreatePlane());
    line->SetControlPoints({ P(10, 10), P(20, 10) });
    CPPUNIT_ASSERT_THROW(line->InsertControlPoint(1, P(15, 10)), mitk::Exception);

    // While being placed, every figure takes appended points up to its maximum.
    auto placedLine = mitk::PlanarLine::New();
    placedLine->SetPlaneGeometry(CreatePlane());
    placedLine->BeginPlacement();
    placedLine->InsertControlPoint(0, P(10, 10));
    placedLine->InsertControlPoint(1, P(20, 10));
    CPPUNIT_ASSERT_EQUAL(2u, placedLine->GetNumberOfControlPoints());
    CPPUNIT_ASSERT_THROW(placedLine->InsertControlPoint(0, P(5, 10)), mitk::Exception);
  }

  void InsertControlPoint_ChecksIndexAndMaximum()
  {
    auto polygon = CreateSquare();
    CPPUNIT_ASSERT_THROW(polygon->InsertControlPoint(5, P(15, 5)), mitk::Exception);

    auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(CreatePlane());
    line->BeginPlacement();
    line->SetControlPoints({ P(10, 10), P(20, 10) });
    CPPUNIT_ASSERT_THROW(line->InsertControlPoint(2, P(30, 10)), mitk::Exception);
  }

  void RemoveControlPoint_WhileBeingPlaced_GoesBelowMinimum()
  {
    auto polygon = CreateSquare();
    polygon->RemoveControlPoint(3);
    polygon->RemoveControlPoint(2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A placed polygon keeps its minimum", 3u, polygon->GetNumberOfControlPoints());

    polygon->BeginPlacement();
    polygon->RemoveControlPoint(2);
    CPPUNIT_ASSERT_EQUAL(2u, polygon->GetNumberOfControlPoints());
  }

  void Translate_MovesEveryControlPoint()
  {
    auto polygon = CreateSquare();

    mitk::Vector2D offset;
    offset[0] = 3.0;
    offset[1] = -2.0;
    polygon->Translate(offset);

    const auto points = polygon->GetControlPoints();
    CPPUNIT_ASSERT(points[0] == P(13, 8));
    CPPUNIT_ASSERT(points[2] == P(23, 18));
  }

  void MoveHandle_OutOfRange_Throws()
  {
    auto polygon = CreateSquare();
    CPPUNIT_ASSERT_THROW(polygon->MoveHandle(4, P(0, 0)), mitk::Exception);

    polygon->MoveHandle(2, P(25, 25));
    CPPUNIT_ASSERT(polygon->GetControlPoint(2) == P(25, 25));
  }

  void Mutators_ModifyFigure()
  {
    auto polygon = CreateSquare();
    mitk::Vector2D offset;
    offset.Fill(1.0);

    CPPUNIT_ASSERT(IsModifiedBy(polygon, [&]() { polygon->SetControlPoints({ P(10, 10), P(20, 10), P(20, 20) }); }));
    CPPUNIT_ASSERT(IsModifiedBy(polygon, [&]() { polygon->InsertControlPoint(3, P(10, 20)); }));
    CPPUNIT_ASSERT(IsModifiedBy(polygon, [&]() { polygon->MoveHandle(0, P(11, 11)); }));
    CPPUNIT_ASSERT(IsModifiedBy(polygon, [&]() { polygon->Translate(offset); }));
    CPPUNIT_ASSERT(IsModifiedBy(polygon, [&]() { polygon->RemoveControlPoint(3); }));
    CPPUNIT_ASSERT(IsModifiedBy(polygon, [&]() { polygon->SetShapeLocked(true); }));
    CPPUNIT_ASSERT(IsModifiedBy(polygon, [&]() { polygon->BeginPlacement(); }));
    CPPUNIT_ASSERT(IsModifiedBy(polygon, [&]() { polygon->EndPlacement(); }));

    auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(CreatePlane());
    CPPUNIT_ASSERT(IsModifiedBy(line, [&]() { line->SetFromSpan(P(10, 10), P(20, 10)); }));
  }

  void Clone_CopiesStateIndependently()
  {
    auto polygon = CreateSquare();
    polygon->SetShapeLocked(true);

    auto clone = polygon->Clone();
    polygon->MoveHandle(0, P(0, 0));

    CPPUNIT_ASSERT(clone->IsShapeLocked());
    CPPUNIT_ASSERT(clone->GetControlPoint(0) == P(10, 10));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigure)

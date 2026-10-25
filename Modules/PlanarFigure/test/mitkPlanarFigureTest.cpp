/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGeometry3D.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarEllipse.h>
#include <mitkPlanarLine.h>
#include <mitkPlanarPath.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlaneGeometry.h>
#include <mitkProportionalTimeGeometry.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <functional>
#include <limits>

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
  MITK_TEST(EndPlacement_TooFewPoints_Throws);
  MITK_TEST(SetControlPoints_ChecksNumber);
  MITK_TEST(SetControlPoints_NotFinite_Throws);
  MITK_TEST(SetControlPoints_WhileBeingPlaced_AcceptsFewerPoints);
  MITK_TEST(InsertControlPoint_OnlyWhereAllowed);
  MITK_TEST(InsertControlPoint_ChecksIndexAndMaximum);
  MITK_TEST(InsertControlPoint_NonPerpendicularAxes_Throws);
  MITK_TEST(RemoveControlPoint_WhileBeingPlaced_GoesBelowMinimum);
  MITK_TEST(Translate_MovesEveryControlPoint);
  MITK_TEST(MoveHandle_OutOfRange_Throws);
  MITK_TEST(Mutators_ModifyFigure);
  MITK_TEST(Clone_CopiesStateIndependently);
  MITK_TEST(Equal_ComparesWhatDefinesTheFigure);
  MITK_TEST(SetTimeGeometry_WithoutPlane_Throws);
  MITK_TEST(FormatValue_TypesetsUnit);

  CPPUNIT_TEST_SUITE_END();

public:
  void NewFigure_HasNoControlPoints()
  {
    auto line = mitk::PlanarLine::New();

    CPPUNIT_ASSERT_EQUAL(std::size_t(0), line->GetNumberOfControlPoints());
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

  void EndPlacement_TooFewPoints_Throws()
  {
    auto polygon = mitk::PlanarPolygon::New();
    polygon->SetPlaneGeometry(CreatePlane());
    polygon->BeginPlacement();
    polygon->SetControlPoints({ P(10, 10), P(20, 10) });

    CPPUNIT_ASSERT_THROW(polygon->EndPlacement(), mitk::Exception);
    CPPUNIT_ASSERT(polygon->IsBeingPlaced());
  }

  void SetControlPoints_ChecksNumber()
  {
    auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(CreatePlane());

    CPPUNIT_ASSERT_THROW(line->SetControlPoints({ P(10, 10) }), mitk::Exception);
    CPPUNIT_ASSERT_THROW(line->SetControlPoints({ P(10, 10), P(20, 10), P(30, 10) }), mitk::Exception);

    line->SetControlPoints({ P(10, 10), P(20, 10) });
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), line->GetNumberOfControlPoints());
    CPPUNIT_ASSERT(line->IsComplete());
  }

  void SetControlPoints_NotFinite_Throws()
  {
    auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(CreatePlane());

    CPPUNIT_ASSERT_THROW(line->SetControlPoints({ P(10, 10), P(std::numeric_limits<double>::quiet_NaN(), 10) }), mitk::Exception);
    CPPUNIT_ASSERT_THROW(line->SetControlPoints({ P(10, std::numeric_limits<double>::infinity()), P(20, 10) }), mitk::Exception);
  }

  void SetControlPoints_WhileBeingPlaced_AcceptsFewerPoints()
  {
    auto polygon = mitk::PlanarPolygon::New();
    polygon->SetPlaneGeometry(CreatePlane());
    polygon->BeginPlacement();
    polygon->SetControlPoints({ P(10, 10) });

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), polygon->GetNumberOfControlPoints());
  }

  void InsertControlPoint_OnlyWhereAllowed()
  {
    auto polygon = CreateSquare();
    polygon->InsertControlPoint(1, P(15, 5));

    CPPUNIT_ASSERT_EQUAL(std::size_t(5), polygon->GetNumberOfControlPoints());
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
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), placedLine->GetNumberOfControlPoints());
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

  void InsertControlPoint_NonPerpendicularAxes_Throws()
  {
    auto ellipse = mitk::PlanarEllipse::New();
    ellipse->SetPlaneGeometry(CreatePlane());
    ellipse->BeginPlacement();
    ellipse->InsertControlPoint(0, P(50, 50));
    ellipse->InsertControlPoint(1, P(60, 50));

    CPPUNIT_ASSERT_THROW(ellipse->InsertControlPoint(2, P(55, 60)), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), ellipse->GetNumberOfControlPoints());
  }

  void RemoveControlPoint_WhileBeingPlaced_GoesBelowMinimum()
  {
    auto polygon = CreateSquare();
    CPPUNIT_ASSERT_THROW(polygon->RemoveControlPoint(4), mitk::Exception);

    polygon->RemoveControlPoint(3);
    CPPUNIT_ASSERT_THROW_MESSAGE("A placed polygon keeps its minimum", polygon->RemoveControlPoint(2), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), polygon->GetNumberOfControlPoints());

    polygon->BeginPlacement();
    polygon->RemoveControlPoint(2);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), polygon->GetNumberOfControlPoints());
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

  void Equal_ComparesWhatDefinesTheFigure()
  {
    const auto square = CreateSquare();
    CPPUNIT_ASSERT(mitk::Equal(*square, *square->Clone(), mitk::eps, true));

    auto moved = square->Clone();
    moved->MoveHandle(0, P(10, 10.001));
    CPPUNIT_ASSERT(!mitk::Equal(*square, *moved, mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Within the tolerance", mitk::Equal(*square, *moved, 0.01, true));

    auto locked = square->Clone();
    locked->SetShapeLocked(true);
    CPPUNIT_ASSERT(!mitk::Equal(*square, *locked, mitk::eps, false));

    auto path = mitk::PlanarPath::New();
    path->SetPlaneGeometry(CreatePlane());
    path->SetControlPoints(square->GetControlPoints());
    CPPUNIT_ASSERT_MESSAGE("Other class", !mitk::Equal(*square, *path, mitk::eps, false));

    auto ring = mitk::PlanarDoubleEllipse::New();
    ring->SetPlaneGeometry(CreatePlane());
    ring->SetFromSpan(P(50, 50), P(70, 50));

    auto thinnerRing = ring->Clone();
    thinnerRing->SetThickness(ring->GetThickness() / 2.0);
    CPPUNIT_ASSERT_MESSAGE("Other thickness", !mitk::Equal(*ring, *thinnerRing, mitk::eps, false));
  }

  void SetTimeGeometry_WithoutPlane_Throws()
  {
    auto line = mitk::PlanarLine::New();
    auto timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(mitk::Geometry3D::New(), 1);

    CPPUNIT_ASSERT_THROW(line->SetTimeGeometry(timeGeometry), mitk::Exception);
  }

  void FormatValue_TypesetsUnit()
  {
    using Type = mitk::PlanarFigure::MeasurementType;

    CPPUNIT_ASSERT_EQUAL(std::string("12.50 mm"), mitk::PlanarFigure::FormatValue({ Type::Length, 12.5 }));
    CPPUNIT_ASSERT_EQUAL(std::string("7.25 mm\xC2\xB2"), mitk::PlanarFigure::FormatValue({ Type::Area, 7.25 }));
    CPPUNIT_ASSERT_EQUAL(std::string("45.00\xC2\xB0"), mitk::PlanarFigure::FormatValue({ Type::Angle, 45.0 }));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigure)

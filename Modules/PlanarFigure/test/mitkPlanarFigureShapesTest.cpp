/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarAngle.h>
#include <mitkPlanarArrow.h>
#include <mitkPlanarBezierCurve.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarCross.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarEllipse.h>
#include <mitkPlanarFourPointAngle.h>
#include <mitkPlanarLine.h>
#include <mitkPlanarPath.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarRectangle.h>
#include <mitkPlanarSubdivisionPolygon.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <cmath>
#include <limits>
#include <numbers>

namespace
{
  constexpr double Epsilon = 1e-9;

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

  template <class TFigure>
  typename TFigure::Pointer Place(typename TFigure::Pointer figure, const std::vector<mitk::Point2D> &points)
  {
    figure->SetPlaneGeometry(CreatePlane());
    figure->SetControlPoints(points);
    return figure;
  }

  double GetMeasurement(const mitk::PlanarFigure *figure, mitk::PlanarFigure::MeasurementType type)
  {
    const auto measurement = figure->GetMeasurement(type);
    CPPUNIT_ASSERT_MESSAGE(mitk::PlanarFigure::GetDisplayName(type), measurement.has_value());
    return *measurement;
  }
}

class mitkPlanarFigureShapesTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureShapesTestSuite);

  MITK_TEST(CreatePlanarFigure_CreatesEveryType);
  MITK_TEST(CreatePlanarFigure_UnknownType_Throws);
  MITK_TEST(SetPlaneGeometry_InvalidPlane_Throws);
  MITK_TEST(SetPlaneGeometry_KeepsCopy);
  MITK_TEST(IsComplete_RequiresPlacement);
  MITK_TEST(Line);
  MITK_TEST(Arrow);
  MITK_TEST(Angle);
  MITK_TEST(FourPointAngle);
  MITK_TEST(Cross);
  MITK_TEST(Circle);
  MITK_TEST(Ellipse_MajorAxisIsTheLongerOne);
  MITK_TEST(DoubleEllipse_HasHole);
  MITK_TEST(Rectangle);
  MITK_TEST(Polygon);
  MITK_TEST(Polygon_SelfIntersecting_HasNoArea);
  MITK_TEST(Path);
  MITK_TEST(SubdivisionPolygon_OpenWhileBeingPlaced);
  MITK_TEST(SubdivisionPolygon_PassesThroughControlPoints);
  MITK_TEST(BezierCurve);
  MITK_TEST(PlacementKinds);
  MITK_TEST(CanInsertControlPoints);
  MITK_TEST(Circle_CenterHandleMovesCircle);
  MITK_TEST(Ellipse_AxesStayPerpendicular);
  MITK_TEST(Ellipse_StartsAsCircle);
  MITK_TEST(Ellipse_KeepsReferenceDirection);
  MITK_TEST(DoubleEllipse_ThicknessStaysValid);
  MITK_TEST(Rectangle_StaysAligned);
  MITK_TEST(Cross_ShortAxisFollowsLongAxis);
  MITK_TEST(Cross_StartsWithDefaultShortAxis);
  MITK_TEST(Cross_LinesKeepCrossing);
  MITK_TEST(CollapsedAxes_PassTheirCheck);
  MITK_TEST(Invariants_AreChecked);

  CPPUNIT_TEST_SUITE_END();

  using MeasurementType = mitk::PlanarFigure::MeasurementType;
  using ContourRole = mitk::PlanarFigure::ContourRole;

public:
  void CreatePlanarFigure_CreatesEveryType()
  {
    for (const std::string typeId : { "line", "arrow", "angle", "four_point_angle", "cross", "circle", "ellipse",
                                      "double_ellipse", "rectangle", "polygon", "path", "subdivision_polygon",
                                      "bezier_curve" })
    {
      const auto figure = mitk::CreatePlanarFigure(typeId);
      CPPUNIT_ASSERT_EQUAL(typeId, figure->GetTypeId());
    }
  }

  void CreatePlanarFigure_UnknownType_Throws()
  {
    CPPUNIT_ASSERT_THROW(mitk::CreatePlanarFigure("square"), mitk::Exception);
  }

  void SetPlaneGeometry_InvalidPlane_Throws()
  {
    auto figure = mitk::PlanarLine::New();
    CPPUNIT_ASSERT_THROW(figure->SetPlaneGeometry(nullptr), mitk::Exception);

#ifdef NDEBUG
    // PlaneGeometry asserts on such bounds in Debug builds.
    auto flatPlane = CreatePlane();
    mitk::BaseGeometry::BoundsArrayType bounds = flatPlane->GetBounds();
    bounds[1] = bounds[0];
    flatPlane->SetBounds(bounds);
    CPPUNIT_ASSERT_THROW(figure->SetPlaneGeometry(flatPlane), mitk::Exception);
#endif
  }

  void SetPlaneGeometry_KeepsCopy()
  {
    auto plane = CreatePlane();
    auto figure = mitk::PlanarLine::New();
    figure->SetPlaneGeometry(plane);

    mitk::Point3D origin;
    origin.Fill(42.0);
    plane->SetOrigin(origin);

    CPPUNIT_ASSERT(figure->GetPlaneGeometry() != plane.GetPointer());
    CPPUNIT_ASSERT(figure->GetPlaneGeometry()->GetOrigin() != origin);
  }

  void IsComplete_RequiresPlacement()
  {
    auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(CreatePlane());
    line->BeginPlacement();
    line->SetFromSpan(P(10, 10), P(10, 10));
    CPPUNIT_ASSERT_MESSAGE("Being placed", !line->IsComplete());

    line->EndPlacement();
    CPPUNIT_ASSERT(line->IsComplete());

    auto angle = mitk::PlanarAngle::New();
    angle->SetPlaneGeometry(CreatePlane());
    angle->BeginPlacement();
    angle->SetControlPoints({ P(10, 10), P(20, 20) });
    CPPUNIT_ASSERT_THROW(angle->EndPlacement(), mitk::Exception);
    CPPUNIT_ASSERT_MESSAGE("Below the minimum number of control points", !angle->IsComplete());
  }

  void Line()
  {
    const auto line = Place<mitk::PlanarLine>(mitk::PlanarLine::New(), { P(10, 10), P(13, 14) });
    const auto contours = line->GetContours();

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), contours.size());
    CPPUNIT_ASSERT(ContourRole::Outline == contours[0].Role);
    CPPUNIT_ASSERT(!contours[0].Closed);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), contours[0].Points.size());
    CPPUNIT_ASSERT(line->GetDecorations().empty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, GetMeasurement(line, MeasurementType::Length), Epsilon);
  }

  void Arrow()
  {
    const auto arrow = Place<mitk::PlanarArrow>(mitk::PlanarArrow::New(), { P(10, 10), P(13, 14) });
    const auto decorations = arrow->GetDecorations();

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), decorations.size());

    const auto *head = std::get_if<mitk::PlanarFigure::ArrowHead>(&decorations[0]);
    CPPUNIT_ASSERT(head != nullptr);
    CPPUNIT_ASSERT(head->Tip == P(10, 10));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, GetMeasurement(arrow, MeasurementType::Length), Epsilon);
  }

  void Angle()
  {
    const auto angle = Place<mitk::PlanarAngle>(mitk::PlanarAngle::New(), { P(20, 10), P(10, 10), P(10, 30) });
    const auto decorations = angle->GetDecorations();

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), angle->GetContours().size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), angle->GetContours()[0].Points.size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), decorations.size());

    const auto *arc = std::get_if<mitk::PlanarFigure::AngleArc>(&decorations[0]);
    CPPUNIT_ASSERT(arc != nullptr);
    CPPUNIT_ASSERT(arc->Vertex == P(10, 10));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(90.0, GetMeasurement(angle, MeasurementType::Angle), Epsilon);
  }

  void FourPointAngle()
  {
    const auto angle = Place<mitk::PlanarFourPointAngle>(mitk::PlanarFourPointAngle::New(),
      { P(10, 10), P(20, 10), P(10, 20), P(20, 30) });

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), angle->GetContours().size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(45.0, GetMeasurement(angle, MeasurementType::Angle), Epsilon);
  }

  void Cross()
  {
    const auto cross = Place<mitk::PlanarCross>(mitk::PlanarCross::New(), { P(10, 20), P(50, 20), P(30, 15), P(30, 25) });

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), cross->GetContours().size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(40.0, GetMeasurement(cross, MeasurementType::LongAxis), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, GetMeasurement(cross, MeasurementType::ShortAxis), Epsilon);
  }

  void Circle()
  {
    const auto circle = Place<mitk::PlanarCircle>(mitk::PlanarCircle::New(), { P(50, 50), P(53, 54) });
    const auto contours = circle->GetContours();

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), contours.size());
    CPPUNIT_ASSERT(contours[0].Closed);

    for (const auto &point : contours[0].Points)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, point.EuclideanDistanceTo(P(50, 50)), Epsilon);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, GetMeasurement(circle, MeasurementType::Radius), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, GetMeasurement(circle, MeasurementType::Diameter), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.0 * std::numbers::pi, GetMeasurement(circle, MeasurementType::Area), Epsilon);
  }

  void Ellipse_MajorAxisIsTheLongerOne()
  {
    // The first axis is the shorter one.
    const auto ellipse = Place<mitk::PlanarEllipse>(mitk::PlanarEllipse::New(), { P(50, 50), P(53, 50), P(50, 60) });
    const auto contours = ellipse->GetContours();

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), contours.size());
    CPPUNIT_ASSERT(contours[0].Closed);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, GetMeasurement(ellipse, MeasurementType::MajorAxis), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, GetMeasurement(ellipse, MeasurementType::MinorAxis), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(30.0 * std::numbers::pi, GetMeasurement(ellipse, MeasurementType::Area), Epsilon);
  }

  void DoubleEllipse_HasHole()
  {
    auto doubleEllipse = mitk::PlanarDoubleEllipse::New();
    doubleEllipse->SetPlaneGeometry(CreatePlane());
    doubleEllipse->SetFromSpan(P(50, 50), P(70, 50));
    doubleEllipse->MoveHandle(2, P(50, 60));
    doubleEllipse->SetThickness(5.0);

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), doubleEllipse->GetNumberOfControlPoints());

    const auto contours = doubleEllipse->GetContours();

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), contours.size());
    CPPUNIT_ASSERT(ContourRole::Outline == contours[0].Role);
    CPPUNIT_ASSERT(ContourRole::Hole == contours[1].Role);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(40.0, GetMeasurement(doubleEllipse, MeasurementType::MajorAxis), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, GetMeasurement(doubleEllipse, MeasurementType::MinorAxis), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, GetMeasurement(doubleEllipse, MeasurementType::Thickness), Epsilon);

    // The inner handle lies opposite of the end of the first axis.
    CPPUNIT_ASSERT(doubleEllipse->GetHandles()[3].Position == P(35, 50));

    CPPUNIT_ASSERT_THROW(doubleEllipse->SetThickness(-1.0), mitk::Exception);
    CPPUNIT_ASSERT_THROW(doubleEllipse->SetThickness(11.0), mitk::Exception);

    // Without radii there is no maximum to clamp to.
    auto withoutRadii = mitk::PlanarDoubleEllipse::New();
    CPPUNIT_ASSERT_THROW(withoutRadii->SetThickness(std::numeric_limits<double>::infinity()), mitk::Exception);

    // A thickness of a whole radius leaves no hole.
    doubleEllipse->SetThickness(10.0);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), doubleEllipse->GetContours().size());
  }

  void Rectangle()
  {
    auto rectangle = mitk::PlanarRectangle::New();
    rectangle->SetPlaneGeometry(CreatePlane());
    rectangle->SetFromSpan(P(10, 10), P(14, 13));

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), rectangle->GetNumberOfControlPoints());

    const auto contours = rectangle->GetContours();

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), contours.size());
    CPPUNIT_ASSERT(contours[0].Closed);
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), contours[0].Points.size());
    CPPUNIT_ASSERT(contours[0].Points[0] == P(10, 10));
    CPPUNIT_ASSERT(contours[0].Points[1] == P(14, 10));
    CPPUNIT_ASSERT(contours[0].Points[2] == P(14, 13));
    CPPUNIT_ASSERT(contours[0].Points[3] == P(10, 13));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.0, GetMeasurement(rectangle, MeasurementType::Circumference), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0, GetMeasurement(rectangle, MeasurementType::Area), Epsilon);
  }

  void Polygon()
  {
    const auto polygon = Place<mitk::PlanarPolygon>(mitk::PlanarPolygon::New(), { P(10, 10), P(20, 10), P(20, 20), P(10, 20) });

    CPPUNIT_ASSERT_EQUAL(std::string("polygon"), polygon->GetTypeId());
    CPPUNIT_ASSERT(polygon->GetContours()[0].Closed);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(40.0, GetMeasurement(polygon, MeasurementType::Circumference), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, GetMeasurement(polygon, MeasurementType::Area), Epsilon);
  }

  void Polygon_SelfIntersecting_HasNoArea()
  {
    const auto bowTie = Place<mitk::PlanarPolygon>(mitk::PlanarPolygon::New(), { P(10, 10), P(20, 20), P(20, 10), P(10, 20) });

    CPPUNIT_ASSERT(bowTie->GetMeasurement(MeasurementType::Circumference).has_value());
    CPPUNIT_ASSERT(!bowTie->GetMeasurement(MeasurementType::Area).has_value());
  }

  void Path()
  {
    auto path = mitk::PlanarPath::New();
    path->SetPlaneGeometry(CreatePlane());
    path->SetControlPoints({ P(10, 10), P(20, 10), P(20, 20) });

    CPPUNIT_ASSERT_EQUAL(std::string("path"), path->GetTypeId());
    CPPUNIT_ASSERT(!path->GetContours()[0].Closed);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), path->GetMeasurements().size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, GetMeasurement(path, MeasurementType::Length), Epsilon);
  }

  void SubdivisionPolygon_OpenWhileBeingPlaced()
  {
    auto polygon = Place<mitk::PlanarSubdivisionPolygon>(mitk::PlanarSubdivisionPolygon::New(),
      { P(10, 10), P(30, 10), P(30, 30), P(10, 30) });

    const auto numberOfCurvePoints = std::size_t(4 << mitk::PlanarSubdivisionPolygon::SubdivisionRounds);

    auto contours = polygon->GetContours();
    CPPUNIT_ASSERT(contours[0].Closed);
    CPPUNIT_ASSERT_EQUAL(numberOfCurvePoints, contours[0].Points.size());
    CPPUNIT_ASSERT(polygon->GetMeasurement(MeasurementType::Area).has_value());

    polygon->BeginPlacement();
    contours = polygon->GetContours();
    CPPUNIT_ASSERT(!contours[0].Closed);
    CPPUNIT_ASSERT(contours[0].Points.back() == P(10, 30));
  }

  void SubdivisionPolygon_PassesThroughControlPoints()
  {
    const std::vector<mitk::Point2D> controlPoints = { P(25, 25), P(75, 25), P(75, 75), P(25, 75) };
    const auto polygon = Place<mitk::PlanarSubdivisionPolygon>(mitk::PlanarSubdivisionPolygon::New(), controlPoints);
    const auto curve = polygon->GetContours()[0].Points;

    // Every control point starts a run of 2^rounds curve points.
    const auto run = std::size_t(1) << mitk::PlanarSubdivisionPolygon::SubdivisionRounds;
    CPPUNIT_ASSERT_EQUAL(4 * run, curve.size());

    for (std::size_t i = 0; i < controlPoints.size(); ++i)
      CPPUNIT_ASSERT(curve[i * run] == controlPoints[i]);

    // Points between them, as computed by the previous implementation
    CPPUNIT_ASSERT(mitk::Equal(curve[10], P(39.624, 19.3268), 1e-3, true));
    CPPUNIT_ASSERT(mitk::Equal(curve[47], P(81.25, 48.243), 1e-3, true));
    CPPUNIT_ASSERT(mitk::Equal(curve[67], P(71.2887, 77.5248), 1e-3, true));
  }

  void BezierCurve()
  {
    const auto curve = Place<mitk::PlanarBezierCurve>(mitk::PlanarBezierCurve::New(), { P(10, 10), P(20, 10), P(30, 10) });
    const auto contours = curve->GetContours();
    const auto decorations = curve->GetDecorations();

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), contours.size());
    CPPUNIT_ASSERT(!contours[0].Closed);
    CPPUNIT_ASSERT_EQUAL(std::size_t(mitk::PlanarBezierCurve::NumberOfSegments + 1), contours[0].Points.size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), decorations.size());
    CPPUNIT_ASSERT(std::holds_alternative<mitk::PlanarFigure::HelperPolyline>(decorations[0]));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, GetMeasurement(curve, MeasurementType::Length), 1e-6);
  }

  void PlacementKinds()
  {
    using PlacementKind = mitk::PlanarFigure::PlacementKind;

    for (const std::string typeId : { "line", "arrow", "cross", "circle", "ellipse", "double_ellipse", "rectangle" })
    {
      auto figure = mitk::CreatePlanarFigure(typeId);
      figure->SetPlaneGeometry(CreatePlane());
      figure->SetFromSpan(P(10, 10), P(20, 15));

      CPPUNIT_ASSERT_MESSAGE(typeId, PlacementKind::Span == figure->GetPlacementKind());
      CPPUNIT_ASSERT_MESSAGE(typeId, figure->IsComplete());
    }

    for (const std::string typeId : { "angle", "four_point_angle", "polygon", "path", "subdivision_polygon", "bezier_curve" })
    {
      auto figure = mitk::CreatePlanarFigure(typeId);

      CPPUNIT_ASSERT_MESSAGE(typeId, PlacementKind::Sequence == figure->GetPlacementKind());
      CPPUNIT_ASSERT_THROW_MESSAGE(typeId, figure->SetFromSpan(P(10, 10), P(20, 15)), mitk::Exception);
    }
  }

  void CanInsertControlPoints()
  {
    for (const std::string typeId : { "polygon", "path" })
      CPPUNIT_ASSERT_MESSAGE(typeId, mitk::CreatePlanarFigure(typeId)->CanInsertControlPoints());

    for (const std::string typeId : { "line", "angle", "circle", "subdivision_polygon", "bezier_curve" })
      CPPUNIT_ASSERT_MESSAGE(typeId, !mitk::CreatePlanarFigure(typeId)->CanInsertControlPoints());
  }

  void Circle_CenterHandleMovesCircle()
  {
    auto circle = mitk::PlanarCircle::New();
    circle->SetPlaneGeometry(CreatePlane());
    circle->SetFromSpan(P(50, 50), P(53, 54));

    const auto handles = circle->GetHandles();
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), handles.size());
    CPPUNIT_ASSERT(mitk::PlanarFigure::HandleRole::Center == handles[0].Role);
    CPPUNIT_ASSERT(mitk::PlanarFigure::HandleRole::Extent == handles[1].Role);

    circle->MoveHandle(0, P(60, 60));
    CPPUNIT_ASSERT(circle->GetControlPoint(1) == P(63, 64));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, GetMeasurement(circle, MeasurementType::Radius), Epsilon);

    circle->MoveHandle(1, P(60, 70));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, GetMeasurement(circle, MeasurementType::Radius), Epsilon);
  }

  void Ellipse_AxesStayPerpendicular()
  {
    auto ellipse = mitk::PlanarEllipse::New();
    ellipse->SetPlaneGeometry(CreatePlane());
    ellipse->SetFromSpan(P(50, 50), P(60, 50));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, GetMeasurement(ellipse, MeasurementType::MinorAxis), Epsilon);

    // The second axis end is projected onto the perpendicular of the first axis.
    ellipse->MoveHandle(2, P(55, 53));
    CPPUNIT_ASSERT(ellipse->GetControlPoint(2) == P(50, 53));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, GetMeasurement(ellipse, MeasurementType::MinorAxis), Epsilon);

    // Turning the first axis turns the second one along.
    ellipse->MoveHandle(1, P(50, 70));
    const auto center = ellipse->GetControlPoint(0);
    const auto firstAxis = ellipse->GetControlPoint(1) - center;
    const auto secondAxis = ellipse->GetControlPoint(2) - center;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, firstAxis * secondAxis, Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(40.0, GetMeasurement(ellipse, MeasurementType::MajorAxis), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, GetMeasurement(ellipse, MeasurementType::MinorAxis), Epsilon);

    ellipse->MoveHandle(0, P(40, 40));
    CPPUNIT_ASSERT(ellipse->GetControlPoint(1) == P(40, 60));
  }

  void Ellipse_StartsAsCircle()
  {
    auto ellipse = mitk::PlanarEllipse::New();
    ellipse->SetPlaneGeometry(CreatePlane());
    ellipse->BeginPlacement();
    ellipse->SetFromSpan(P(50, 50), P(60, 50));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, GetMeasurement(ellipse, MeasurementType::MajorAxis), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, GetMeasurement(ellipse, MeasurementType::MinorAxis), Epsilon);
  }

  void Ellipse_KeepsReferenceDirection()
  {
    auto ellipse = mitk::PlanarEllipse::New();
    ellipse->SetPlaneGeometry(CreatePlane());
    ellipse->SetFromSpan(P(50, 50), P(60, 50));
    CPPUNIT_ASSERT(!ellipse->GetReferenceDirection().has_value());

    mitk::Vector2D direction;
    direction[0] = 0.0;
    direction[1] = 2.0;
    ellipse->SetReferenceDirection(direction);

    ellipse->MoveHandle(1, P(50, 70));
    ellipse->MoveHandle(0, P(40, 40));

    const auto referenceDirection = ellipse->Clone()->GetReferenceDirection();
    CPPUNIT_ASSERT(referenceDirection.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, (*referenceDirection)[0], Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, (*referenceDirection)[1], Epsilon);

    direction.Fill(0.0);
    CPPUNIT_ASSERT_THROW(ellipse->SetReferenceDirection(direction), mitk::Exception);

    direction[0] = std::numeric_limits<double>::quiet_NaN();
    CPPUNIT_ASSERT_THROW(ellipse->SetReferenceDirection(direction), mitk::Exception);

    direction[0] = std::numeric_limits<double>::infinity();
    CPPUNIT_ASSERT_THROW(ellipse->SetReferenceDirection(direction), mitk::Exception);
  }

  void DoubleEllipse_ThicknessStaysValid()
  {
    auto ring = mitk::PlanarDoubleEllipse::New();
    ring->SetPlaneGeometry(CreatePlane());
    ring->SetFromSpan(P(50, 50), P(70, 50));

    CPPUNIT_ASSERT_EQUAL(std::size_t(4), ring->GetHandles().size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, GetMeasurement(ring, MeasurementType::Thickness), Epsilon);

    ring->MoveHandle(3, P(49, 50));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(19.0, GetMeasurement(ring, MeasurementType::Thickness), Epsilon);

    // The minor axis cannot become shorter than the ring is thick.
    ring->MoveHandle(2, P(50, 55));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(38.0, GetMeasurement(ring, MeasurementType::MinorAxis), Epsilon);

    // The ring cannot become thicker than the minor radius.
    ring->MoveHandle(3, P(50, 50));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(19.0, GetMeasurement(ring, MeasurementType::Thickness), Epsilon);

    // Shortening the first axis thins the ring.
    ring->MoveHandle(1, P(60, 50));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, ring->GetThickness(), Epsilon);

    // The second axis may then stay longer than the first, so its handle does not jump.
    ring->MoveHandle(2, P(50, 69.1));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(38.2, GetMeasurement(ring, MeasurementType::MajorAxis), Epsilon);
  }

  void Rectangle_StaysAligned()
  {
    auto rectangle = mitk::PlanarRectangle::New();
    rectangle->SetPlaneGeometry(CreatePlane());
    rectangle->SetFromSpan(P(10, 10), P(20, 15));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(50.0, GetMeasurement(rectangle, MeasurementType::Area), Epsilon);
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), rectangle->GetHandles().size());

    rectangle->MoveHandle(2, P(30, 25));
    auto handles = rectangle->GetHandles();
    CPPUNIT_ASSERT(handles[1].Position == P(30, 10));
    CPPUNIT_ASSERT(handles[3].Position == P(10, 25));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(300.0, GetMeasurement(rectangle, MeasurementType::Area), Epsilon);

    // Moving a derived corner keeps the opposite one in place.
    rectangle->MoveHandle(1, P(40, 5));
    handles = rectangle->GetHandles();
    CPPUNIT_ASSERT(handles[0].Position == P(10, 5));
    CPPUNIT_ASSERT(handles[2].Position == P(40, 25));
    CPPUNIT_ASSERT(handles[3].Position == P(10, 25));

    rectangle->MoveHandle(3, P(0, 30));
    handles = rectangle->GetHandles();
    CPPUNIT_ASSERT(handles[0].Position == P(0, 5));
    CPPUNIT_ASSERT(handles[1].Position == P(40, 5));
    CPPUNIT_ASSERT(handles[2].Position == P(40, 30));
  }

  void Cross_ShortAxisFollowsLongAxis()
  {
    auto cross = mitk::PlanarCross::New();
    cross->SetPlaneGeometry(CreatePlane());
    cross->SetFromSpan(P(10, 20), P(50, 20));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(40.0, GetMeasurement(cross, MeasurementType::LongAxis), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, GetMeasurement(cross, MeasurementType::ShortAxis), Epsilon);

    cross->MoveHandle(1, P(10, 60));
    const auto longAxis = cross->GetControlPoint(1) - cross->GetControlPoint(0);
    const auto shortAxis = cross->GetControlPoint(3) - cross->GetControlPoint(2);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, longAxis * shortAxis, Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, GetMeasurement(cross, MeasurementType::ShortAxis), Epsilon);

    // The short axis keeps crossing the long axis.
    cross->MoveHandle(2, P(30, 100));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(60.0, cross->GetControlPoint(2)[1], Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(60.0, cross->GetControlPoint(3)[1], Epsilon);
  }

  void Cross_LinesKeepCrossing()
  {
    auto cross = mitk::PlanarCross::New();
    cross->SetPlaneGeometry(CreatePlane());
    cross->SetFromSpan(P(10, 20), P(50, 20));

    // An end of the second line cannot pass to the side of its other end.
    const auto otherEnd = cross->GetControlPoint(3);
    cross->MoveHandle(2, P(30, otherEnd[1] - 5.0));
    CPPUNIT_ASSERT(cross->GetControlPoint(2) == P(30, 20));
  }

  void CollapsedAxes_PassTheirCheck()
  {
    // A collapsed axis is left with a length of rounding noise in any direction,
    // which the reader takes through SetControlPoints().
    for (int i = 0; i < 360; ++i)
    {
      const auto angle = 0.1 + 0.0137 * i;
      const auto start = P(0.3, 0.7);
      const auto end = P(start[0] + 70.0 * std::cos(angle), start[1] + 70.0 * std::sin(angle));
      const auto middle = start + (end - start) * 0.5;

      auto ellipse = mitk::PlanarEllipse::New();
      ellipse->SetPlaneGeometry(CreatePlane());
      ellipse->SetFromSpan(start, end);
      ellipse->MoveHandle(2, end);
      CPPUNIT_ASSERT_NO_THROW_MESSAGE("Ellipse", mitk::PlanarEllipse::New()->SetControlPoints(ellipse->GetControlPoints()));

      auto cross = mitk::PlanarCross::New();
      cross->SetPlaneGeometry(CreatePlane());
      cross->SetFromSpan(start, end);
      cross->MoveHandle(2, middle);
      cross->MoveHandle(3, middle);
      CPPUNIT_ASSERT_NO_THROW_MESSAGE("Cross", mitk::PlanarCross::New()->SetControlPoints(cross->GetControlPoints()));
    }
  }

  void Invariants_AreChecked()
  {
    auto ellipse = mitk::PlanarEllipse::New();
    CPPUNIT_ASSERT_THROW(ellipse->SetControlPoints({ P(50, 50), P(60, 50), P(55, 60) }), mitk::Exception);
    ellipse->SetControlPoints({ P(50, 50), P(60, 50), P(50, 55) });

    auto doubleEllipse = mitk::PlanarDoubleEllipse::New();
    CPPUNIT_ASSERT_THROW(doubleEllipse->SetControlPoints({ P(50, 50), P(60, 50), P(55, 60) }), mitk::Exception);
    doubleEllipse->SetControlPoints({ P(50, 50), P(60, 50), P(50, 55) });

    auto cross = mitk::PlanarCross::New();
    CPPUNIT_ASSERT_THROW_MESSAGE("Not perpendicular",
      cross->SetControlPoints({ P(10, 20), P(50, 20), P(30, 10), P(35, 30) }), mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("Beyond the first line",
      cross->SetControlPoints({ P(10, 20), P(50, 20), P(60, 10), P(60, 30) }), mitk::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("On one side of the first line",
      cross->SetControlPoints({ P(10, 20), P(50, 20), P(30, 25), P(30, 30) }), mitk::Exception);

    // Touching the first line counts as crossing it.
    cross->SetControlPoints({ P(10, 20), P(50, 20), P(30, 10), P(30, 20) });
  }

  void Cross_StartsWithDefaultShortAxis()
  {
    auto cross = mitk::PlanarCross::New();
    cross->SetPlaneGeometry(CreatePlane());
    cross->BeginPlacement();
    cross->SetFromSpan(P(10, 20), P(50, 20));

    CPPUNIT_ASSERT_EQUAL(std::size_t(4), cross->GetNumberOfControlPoints());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(40.0, GetMeasurement(cross, MeasurementType::LongAxis), Epsilon);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, GetMeasurement(cross, MeasurementType::ShortAxis), Epsilon);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureShapes)

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarFigure.h>

#include <mitkAbstractTransformGeometry.h>
#include <mitkExceptionMacro.h>
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
#include <mitkProportionalTimeGeometry.h>

#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <typeinfo>

namespace
{
  void CheckPlane(const mitk::BaseGeometry *geometry)
  {
    const auto *plane = dynamic_cast<const mitk::PlaneGeometry *>(geometry);

    if (nullptr == plane)
      mitkThrow() << "A planar figure needs a plane.";

    if (nullptr != dynamic_cast<const mitk::AbstractTransformGeometry *>(plane))
      mitkThrow() << "A planar figure cannot lie on a curved plane.";

    // Also catches a singular transform, which leaves the plane without an area in world coordinates.
    if (!(itk::CrossProduct(plane->GetAxisVector(0), plane->GetAxisVector(1)).GetNorm() > 0.0))
      mitkThrow() << "The plane of a planar figure needs an extent.";
  }

  void CheckTimeGeometry(const mitk::TimeGeometry *geometry)
  {
    if (nullptr == geometry || 1 != geometry->CountTimeSteps())
      mitkThrow() << "A planar figure needs a geometry of a single time step.";

    CheckPlane(geometry->GetGeometryForTimeStep(0));
  }
}

mitk::PlanarFigure::PlanarFigure()
{
  this->InitializeTimeGeometry(1);
}

void mitk::PlanarFigure::SetPlaneGeometry(const PlaneGeometry *geometry)
{
  CheckPlane(geometry);
  this->SetGeometry(geometry->Clone());
}

const mitk::PlaneGeometry *mitk::PlanarFigure::GetPlaneGeometry() const
{
  return dynamic_cast<const PlaneGeometry *>(this->GetGeometry(0));
}

void mitk::PlanarFigure::SetTimeGeometry(TimeGeometry *geometry)
{
  CheckTimeGeometry(geometry);
  Superclass::SetTimeGeometry(geometry);
}

void mitk::PlanarFigure::SetClonedGeometry(const BaseGeometry *geometry, unsigned int time)
{
  if (0 != time)
    mitkThrow() << "A planar figure has a single time step.";

  CheckPlane(geometry);
  Superclass::SetClonedGeometry(geometry, time);
}

void mitk::PlanarFigure::CopyInformation(const itk::DataObject *data)
{
  // The superclass takes the time geometry of the data without a check.
  if (const auto *other = dynamic_cast<const BaseData *>(data); nullptr != other && nullptr != other->GetTimeGeometry())
    CheckTimeGeometry(other->GetTimeGeometry());

  Superclass::CopyInformation(data);
}

std::string mitk::PlanarFigure::GetDisplayName(MeasurementType type)
{
  switch (type)
  {
    case MeasurementType::Length:
      return "Length";
    case MeasurementType::Circumference:
      return "Circumference";
    case MeasurementType::Area:
      return "Area";
    case MeasurementType::Angle:
      return "Angle";
    case MeasurementType::Radius:
      return "Radius";
    case MeasurementType::Diameter:
      return "Diameter";
    case MeasurementType::MajorAxis:
      return "Major Axis";
    case MeasurementType::MinorAxis:
      return "Minor Axis";
    case MeasurementType::Thickness:
      return "Thickness";
    case MeasurementType::LongAxis:
      return "Longest Axis";
    case MeasurementType::ShortAxis:
      return "Short Axis";
  }

  mitkThrow() << "Unknown measurement type.";
}

std::string mitk::PlanarFigure::GetUnitSymbol(MeasurementType type)
{
  switch (type)
  {
    case MeasurementType::Area:
      return "mm2";
    case MeasurementType::Angle:
      return "deg";
    default:
      return "mm";
  }
}

std::string mitk::PlanarFigure::FormatValue(const Measurement &measurement)
{
  std::ostringstream stream;
  stream.imbue(std::locale::classic());
  stream << std::fixed << std::setprecision(2) << measurement.Value;

  switch (measurement.Type)
  {
    case MeasurementType::Area:
      stream << " mm\xC2\xB2";
      break;
    case MeasurementType::Angle:
      stream << "\xC2\xB0";
      break;
    default:
      stream << ' ' << GetUnitSymbol(measurement.Type);
      break;
  }

  return stream.str();
}

std::vector<mitk::Point2D> mitk::PlanarFigure::GetControlPoints() const
{
  return m_ControlPoints;
}

std::size_t mitk::PlanarFigure::GetNumberOfControlPoints() const
{
  return m_ControlPoints.size();
}

mitk::Point2D mitk::PlanarFigure::GetControlPoint(std::size_t index) const
{
  if (index >= m_ControlPoints.size())
    mitkThrow() << "Control point index " << index << " is out of range.";

  return m_ControlPoints[index];
}

mitk::Point3D mitk::PlanarFigure::GetWorldControlPoint(std::size_t index) const
{
  Point3D worldPoint;
  this->GetPlaneGeometry()->Map(this->GetControlPoint(index), worldPoint);
  return worldPoint;
}

void mitk::PlanarFigure::SetControlPoints(const std::vector<Point2D> &points)
{
  this->CheckControlPoints(points);
  this->AssignControlPoints(points);
}

void mitk::PlanarFigure::CheckControlPoints(const std::vector<Point2D> &points) const
{
  if (points.size() > this->GetMaximumNumberOfControlPoints())
    mitkThrow() << this->GetNameOfClass() << " takes at most " << this->GetMaximumNumberOfControlPoints() << " control points.";

  if (!m_IsBeingPlaced && points.size() < this->GetMinimumNumberOfControlPoints())
    mitkThrow() << this->GetNameOfClass() << " needs at least " << this->GetMinimumNumberOfControlPoints() << " control points.";

  for (const auto &point : points)
  {
    if (!std::isfinite(point[0]) || !std::isfinite(point[1]))
      mitkThrow() << "The control points of a planar figure must be finite.";
  }
}

void mitk::PlanarFigure::AssignControlPoints(const std::vector<Point2D> &points)
{
  m_ControlPoints = points;
  this->Modified();
}

void mitk::PlanarFigure::ReplaceControlPoint(std::size_t index, const Point2D &point)
{
  if (index >= m_ControlPoints.size())
    mitkThrow() << "Control point index " << index << " is out of range.";

  m_ControlPoints[index] = point;
  this->Modified();
}

bool mitk::PlanarFigure::CanInsertControlPoints() const
{
  return false;
}

void mitk::PlanarFigure::InsertControlPoint(std::size_t index, const Point2D &point)
{
  const auto numberOfControlPoints = m_ControlPoints.size();

  if (!this->CanInsertControlPoints() && !(m_IsBeingPlaced && index == numberOfControlPoints))
    mitkThrow() << this->GetNameOfClass() << " cannot insert control points.";

  if (index > numberOfControlPoints)
    mitkThrow() << "Control point index " << index << " is out of range.";

  auto points = m_ControlPoints;
  points.insert(points.begin() + index, point);
  this->SetControlPoints(points);
}

void mitk::PlanarFigure::RemoveControlPoint(std::size_t index)
{
  if (!this->CanInsertControlPoints() && !m_IsBeingPlaced)
    mitkThrow() << this->GetNameOfClass() << " cannot remove control points.";

  if (index >= m_ControlPoints.size())
    mitkThrow() << "Control point index " << index << " is out of range.";

  auto points = m_ControlPoints;
  points.erase(points.begin() + index);
  this->SetControlPoints(points);
}

std::vector<mitk::PlanarFigure::Handle> mitk::PlanarFigure::GetHandles() const
{
  std::vector<Handle> handles;

  for (const auto &point : m_ControlPoints)
    handles.push_back({ HandleRole::Vertex, point });

  return handles;
}

void mitk::PlanarFigure::MoveHandle(std::size_t index, const Point2D &position)
{
  this->ReplaceControlPoint(index, position);
}

void mitk::PlanarFigure::Translate(const Vector2D &offset)
{
  for (auto &point : m_ControlPoints)
    point += offset;

  this->Modified();
}

void mitk::PlanarFigure::SetFromSpan(const Point2D &, const Point2D &)
{
  mitkThrow() << this->GetNameOfClass() << " is not placed by a span.";
}

// The contours of some types differ while the figure is placed.
void mitk::PlanarFigure::BeginPlacement()
{
  m_IsBeingPlaced = true;
  this->Modified();
}

void mitk::PlanarFigure::EndPlacement()
{
  // The other constraints hold already: every change of the points checks them or keeps them consistent.
  if (m_ControlPoints.size() < this->GetMinimumNumberOfControlPoints())
    mitkThrow() << this->GetNameOfClass() << " needs at least " << this->GetMinimumNumberOfControlPoints() << " control points.";

  m_IsBeingPlaced = false;
  this->Modified();
}

bool mitk::PlanarFigure::IsBeingPlaced() const
{
  return m_IsBeingPlaced;
}

bool mitk::PlanarFigure::IsComplete() const
{
  return !m_IsBeingPlaced && m_ControlPoints.size() >= this->GetMinimumNumberOfControlPoints();
}

void mitk::PlanarFigure::SetShapeLocked(bool locked)
{
  if (locked == m_IsShapeLocked)
    return;

  m_IsShapeLocked = locked;
  this->Modified();
}

bool mitk::PlanarFigure::IsShapeLocked() const
{
  return m_IsShapeLocked;
}

std::vector<mitk::PlanarFigure::Decoration> mitk::PlanarFigure::GetDecorations() const
{
  return {};
}

std::optional<double> mitk::PlanarFigure::GetMeasurement(MeasurementType type) const
{
  for (const auto &measurement : this->GetMeasurements())
  {
    if (measurement.Type == type)
      return measurement.Value;
  }

  return std::nullopt;
}

bool mitk::PlanarFigure::IsEmptyTimeStep(unsigned int) const
{
  return m_ControlPoints.empty();
}

bool mitk::PlanarFigure::EqualShape(const PlanarFigure &, ScalarType, bool) const
{
  return true;
}

void mitk::PlanarFigure::UpdateOutputInformation()
{
  // The plane defines the bounds of the figure; they are not computed from its points.
  Superclass::UpdateOutputInformation();
  this->GetTimeGeometry()->Update();
}

void mitk::PlanarFigure::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::PlanarFigure::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::PlanarFigure::VerifyRequestedRegion()
{
  return true;
}

void mitk::PlanarFigure::SetRequestedRegion(const itk::DataObject *)
{
}

void mitk::PlanarFigure::InitializeTimeGeometry(TimeStepType timeSteps)
{
  auto plane = PlaneGeometry::New();
  plane->Initialize();

  auto timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(plane, timeSteps);
  this->SetTimeGeometry(timeGeometry);
}

void mitk::PlanarFigure::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Type: " << this->GetTypeId() << std::endl;
  os << indent << "Being placed: " << m_IsBeingPlaced << std::endl;
  os << indent << "Shape locked: " << m_IsShapeLocked << std::endl;
  os << indent << "Control points:" << std::endl;

  for (std::size_t i = 0; i < m_ControlPoints.size(); ++i)
    os << indent.GetNextIndent() << i << ": " << m_ControlPoints[i] << std::endl;
}

bool mitk::Equal(const PlanarFigure &leftHandSide, const PlanarFigure &rightHandSide, ScalarType eps, bool verbose)
{
  if (typeid(leftHandSide) != typeid(rightHandSide))
  {
    if (verbose)
      MITK_INFO << "[Equal( PlanarFigure )] Classes differ: " << leftHandSide.GetNameOfClass() << " and " << rightHandSide.GetNameOfClass() << ".";

    return false;
  }

  if (leftHandSide.IsBeingPlaced() != rightHandSide.IsBeingPlaced() ||
      leftHandSide.IsShapeLocked() != rightHandSide.IsShapeLocked())
  {
    if (verbose)
      MITK_INFO << "[Equal( PlanarFigure )] Placement or shape lock differ.";

    return false;
  }

  if (!Equal(*leftHandSide.GetPlaneGeometry(), *rightHandSide.GetPlaneGeometry(), eps, verbose))
  {
    if (verbose)
      MITK_INFO << "[Equal( PlanarFigure )] Planes differ.";

    return false;
  }

  const auto leftPoints = leftHandSide.GetControlPoints();
  const auto rightPoints = rightHandSide.GetControlPoints();

  if (leftPoints.size() != rightPoints.size())
  {
    if (verbose)
      MITK_INFO << "[Equal( PlanarFigure )] Numbers of control points differ: " << leftPoints.size() << " and " << rightPoints.size() << ".";

    return false;
  }

  for (std::size_t i = 0; i < leftPoints.size(); ++i)
  {
    if (!Equal(leftPoints[i], rightPoints[i], eps, verbose))
    {
      if (verbose)
        MITK_INFO << "[Equal( PlanarFigure )] Control points " << i << " differ.";

      return false;
    }
  }

  return leftHandSide.EqualShape(rightHandSide, eps, verbose);
}

mitk::PlanarFigure::Pointer mitk::CreatePlanarFigure(const std::string &typeId)
{
  if (typeId == "line")
    return PlanarLine::New().GetPointer();

  if (typeId == "arrow")
    return PlanarArrow::New().GetPointer();

  if (typeId == "angle")
    return PlanarAngle::New().GetPointer();

  if (typeId == "four_point_angle")
    return PlanarFourPointAngle::New().GetPointer();

  if (typeId == "cross")
    return PlanarCross::New().GetPointer();

  if (typeId == "circle")
    return PlanarCircle::New().GetPointer();

  if (typeId == "ellipse")
    return PlanarEllipse::New().GetPointer();

  if (typeId == "double_ellipse")
    return PlanarDoubleEllipse::New().GetPointer();

  if (typeId == "rectangle")
    return PlanarRectangle::New().GetPointer();

  if (typeId == "polygon")
    return PlanarPolygon::New().GetPointer();

  if (typeId == "path")
    return PlanarPath::New().GetPointer();

  if (typeId == "subdivision_polygon")
    return PlanarSubdivisionPolygon::New().GetPointer();

  if (typeId == "bezier_curve")
    return PlanarBezierCurve::New().GetPointer();

  mitkThrow() << "Unknown planar figure type \"" << typeId << "\".";
}

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarCircle.h"
#include "mitkPlaneGeometry.h"

#include "mitkProperties.h"

mitk::PlanarCircle::PlanarCircle()
  : FEATURE_ID_RADIUS(this->AddFeature("Radius", "mm")),
    FEATURE_ID_DIAMETER(this->AddFeature("Diameter", "mm")),
    FEATURE_ID_AREA(this->AddFeature("Area", "mm2")),
    m_MinRadius(0),
    m_MaxRadius(100),
    m_MinMaxRadiusContraintsActive(false),
    m_RadiusFixed(false)
{
  // Circle has two control points
  this->ResetNumberOfControlPoints(2);
  this->SetNumberOfPolyLines(1);
  this->SetProperty("closed", mitk::BoolProperty::New(true));
}

mitk::PlanarCircle::PlanarCircle(double fixedRadius)
  : FEATURE_ID_RADIUS(this->AddFeature("Radius", "mm")),
  FEATURE_ID_DIAMETER(this->AddFeature("Diameter", "mm")),
  FEATURE_ID_AREA(this->AddFeature("Area", "mm2")),
  m_MinRadius(fixedRadius),
  m_MaxRadius(100),
  m_MinMaxRadiusContraintsActive(false),
  m_RadiusFixed(true)
{
  // Fixed Circle has 1 control points
  this->ResetNumberOfControlPoints(1);
  this->SetNumberOfPolyLines(1);
  this->SetProperty("closed", mitk::BoolProperty::New(true));
}

bool mitk::PlanarCircle::SetControlPoint(unsigned int index, const Point2D &point, bool /*createIfDoesNotExist*/)
{
  // moving center point
  if (index == 0)
  {
    const Point2D &centerPoint = GetControlPoint(0);
    if (!m_RadiusFixed)
    {
      Point2D boundaryPoint = GetControlPoint(1);
      const vnl_vector<ScalarType> vec = (point.GetVnlVector() - centerPoint.GetVnlVector());

      boundaryPoint[0] += vec[0];
      boundaryPoint[1] += vec[1];
      PlanarFigure::SetControlPoint(1, boundaryPoint);
    }
    PlanarFigure::SetControlPoint(0, point);
    return true;
  }
  else if (index == 1)
  {
    PlanarFigure::SetControlPoint(index, point);
    return true;
  }
  return false;
}

mitk::Point2D mitk::PlanarCircle::ApplyControlPointConstraints(unsigned int index, const Point2D &point)
{
  if (this->GetPlaneGeometry() == nullptr)
  {
    return point;
  }

  Point2D indexPoint;
  this->GetPlaneGeometry()->WorldToIndex(point, indexPoint);

  const BoundingBox::BoundsArrayType bounds = this->GetPlaneGeometry()->GetBounds();
  if (indexPoint[0] < bounds[0])
  {
    indexPoint[0] = bounds[0];
  }
  if (indexPoint[0] > bounds[1])
  {
    indexPoint[0] = bounds[1];
  }
  if (indexPoint[1] < bounds[2])
  {
    indexPoint[1] = bounds[2];
  }
  if (indexPoint[1] > bounds[3])
  {
    indexPoint[1] = bounds[3];
  }

  Point2D constrainedPoint;
  this->GetPlaneGeometry()->IndexToWorld(indexPoint, constrainedPoint);

  if (m_MinMaxRadiusContraintsActive)
  {
    if (index != 0)
    {
      const Point2D &centerPoint = this->GetControlPoint(0);
      const double euclideanDinstanceFromCenterToPoint1 = centerPoint.EuclideanDistanceTo(point);

      Vector2D vectorProjectedPoint = point - centerPoint;
      vectorProjectedPoint.Normalize();

      if (euclideanDinstanceFromCenterToPoint1 > m_MaxRadius)
      {
        vectorProjectedPoint *= m_MaxRadius;
        constrainedPoint = centerPoint;
        constrainedPoint += vectorProjectedPoint;
      }
      else if (euclideanDinstanceFromCenterToPoint1 < m_MinRadius)
      {
        vectorProjectedPoint *= m_MinRadius;
        constrainedPoint = centerPoint;
        constrainedPoint += vectorProjectedPoint;
      }
    }
  }

  return constrainedPoint;
}

void mitk::PlanarCircle::GeneratePolyLine()
{
  // TODO: start circle at specified boundary point...

  // clear the PolyLine-Contrainer, it will be reconstructed soon enough...
  this->ClearPolyLines();

  const Point2D &centerPoint = GetControlPoint(0);

  double radius = m_MinRadius;
    
  if (!m_RadiusFixed)
  {
    const Point2D &boundaryPoint = GetControlPoint(1);
    radius = centerPoint.EuclideanDistanceTo(boundaryPoint);
  }

  // Generate poly-line with 64 segments
  for (int t = 0; t < 64; ++t)
  {
    const double alpha = (double)t * vnl_math::pi / 32.0;

    // construct the new polyline point ...
    Point2D polyLinePoint;
    polyLinePoint[0] = centerPoint[0] + radius * cos(alpha);
    polyLinePoint[1] = centerPoint[1] + radius * sin(alpha);

    // ... and append it to the PolyLine.
    // No extending supported here, so we can set the index of the PolyLineElement to '0'
    this->AppendPointToPolyLine(0, polyLinePoint);
  }
}

void mitk::PlanarCircle::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
  // A circle does not require a helper object
}

void mitk::PlanarCircle::EvaluateFeaturesInternal()
{
  // Calculate circle radius and area
  const Point3D &p0 = this->GetWorldControlPoint(0);
  double radius = m_MinRadius;

  if (!m_RadiusFixed)
  {
    const Point3D &p1 = this->GetWorldControlPoint(1);
    radius = p0.EuclideanDistanceTo(p1);
  }

  const double area = vnl_math::pi * radius * radius;

  this->SetQuantity(FEATURE_ID_RADIUS, radius);
  this->SetQuantity(FEATURE_ID_DIAMETER, 2 * radius);
  this->SetQuantity(FEATURE_ID_AREA, area);
}

void mitk::PlanarCircle::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

bool mitk::PlanarCircle::SetCurrentControlPoint(const Point2D &point)
{
  if (m_SelectedControlPoint < 0)
  {
    m_SelectedControlPoint = 1;
  }

  return this->SetControlPoint(m_SelectedControlPoint, point, false);
}

bool mitk::PlanarCircle::Equals(const PlanarFigure &other) const
{
  const auto *otherCircle = dynamic_cast<const mitk::PlanarCircle *>(&other);
  if (otherCircle)
  {
    return Superclass::Equals(other);
  }
  else
  {
    return false;
  }
}

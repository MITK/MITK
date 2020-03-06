/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarDoubleEllipse.h"
#include <mitkProperties.h>

#include <algorithm>

mitk::PlanarDoubleEllipse::PlanarDoubleEllipse()
  : FEATURE_ID_MAJOR_AXIS(Superclass::AddFeature("Major Axis", "mm")),
    FEATURE_ID_MINOR_AXIS(Superclass::AddFeature("Minor Axis", "mm")),
    FEATURE_ID_THICKNESS(Superclass::AddFeature("Thickness", "mm")),
    m_NumberOfSegments(64),
    m_ConstrainCircle(true),
    m_ConstrainThickness(true)
{
  this->ResetNumberOfControlPoints(4);
  this->SetNumberOfPolyLines(2);
  this->SetProperty("closed", mitk::BoolProperty::New(true));
}

mitk::PlanarDoubleEllipse::PlanarDoubleEllipse(double fixedRadius, double fixedThickness)
  : FEATURE_ID_MAJOR_AXIS(Superclass::AddFeature("Major Axis", "mm")),
  FEATURE_ID_MINOR_AXIS(Superclass::AddFeature("Minor Axis", "mm")),
  FEATURE_ID_THICKNESS(Superclass::AddFeature("Thickness", "mm")),
  m_NumberOfSegments(64),
  m_ConstrainCircle(true),
  m_ConstrainThickness(true),
  m_FixedRadius(fixedRadius),
  m_FixedThickness(fixedThickness),
  m_SizeIsFixed(true)
{
  this->ResetNumberOfControlPoints(1);
  this->SetNumberOfPolyLines(2);
  this->SetProperty("closed", mitk::BoolProperty::New(true));

  if (fixedThickness>fixedRadius)
  {
    mitkThrow() << "Invalid constructor of fixed sized double ellipses. Thickness (" << fixedThickness << ") is greater than the radius (" << fixedRadius << ")";
  }
}

mitk::Point2D mitk::PlanarDoubleEllipse::ApplyControlPointConstraints(unsigned int index, const Point2D &point)
{
  if (index == 2 && !m_ConstrainCircle)
  {
    const Point2D centerPoint = this->GetControlPoint(CP_CENTER);
    const Vector2D outerMajorVector = this->GetControlPoint(CP_OUTER_MAJOR_AXIS) - centerPoint;

    Vector2D minorDirection;
    minorDirection[0] = outerMajorVector[1];
    minorDirection[1] = -outerMajorVector[0];
    minorDirection.Normalize();

    const double outerMajorRadius = outerMajorVector.GetNorm();
    const double innerMajorRadius = (this->GetControlPoint(CP_INNER_MAJOR_AXIS) - centerPoint).GetNorm();
    const ScalarType radius =
      std::max(outerMajorRadius - innerMajorRadius, std::min(centerPoint.EuclideanDistanceTo(point), outerMajorRadius));

    return centerPoint + minorDirection * radius;
  }
  else if (index == 3 && !m_ConstrainThickness)
  {
    const Point2D centerPoint = this->GetControlPoint(CP_CENTER);
    Vector2D outerMajorVector = this->GetControlPoint(CP_OUTER_MAJOR_AXIS) - centerPoint;

    const double outerMajorRadius = outerMajorVector.GetNorm();
    const double outerMinorRadius = (this->GetControlPoint(CP_OUTER_MINOR_AXIS) - centerPoint).GetNorm();
    const ScalarType radius =
      std::max(outerMajorRadius - outerMinorRadius, std::min(centerPoint.EuclideanDistanceTo(point), outerMajorRadius));

    outerMajorVector.Normalize();

    return centerPoint - outerMajorVector * radius;
  }

  return point;
}

void mitk::PlanarDoubleEllipse::EvaluateFeaturesInternal()
{
  const Point2D centerPoint = this->GetControlPoint(CP_CENTER);
  const ScalarType outerMajorRadius = (m_SizeIsFixed)? m_FixedRadius : centerPoint.EuclideanDistanceTo(this->GetControlPoint(CP_OUTER_MAJOR_AXIS));
  const ScalarType outerMinorRadius = (m_SizeIsFixed)? m_FixedRadius : centerPoint.EuclideanDistanceTo(this->GetControlPoint(CP_OUTER_MINOR_AXIS));
  const ScalarType thickness = (m_SizeIsFixed)? m_FixedThickness : outerMajorRadius - centerPoint.EuclideanDistanceTo(this->GetControlPoint(CP_INNER_MAJOR_AXIS));

  this->SetQuantity(FEATURE_ID_MAJOR_AXIS, 2 * outerMajorRadius);
  this->SetQuantity(FEATURE_ID_MINOR_AXIS, 2 * outerMinorRadius);
  this->SetQuantity(FEATURE_ID_THICKNESS, thickness);
}

void mitk::PlanarDoubleEllipse::GenerateHelperPolyLine(double, unsigned int)
{
}

void mitk::PlanarDoubleEllipse::GeneratePolyLine()
{
  this->ClearPolyLines();

  const Point2D centerPoint = this->GetControlPoint(CP_CENTER);

  Vector2D direction(0.);
  direction[0] = 1.;
  if (!m_SizeIsFixed)
  {
    direction = this->GetControlPoint(CP_OUTER_MAJOR_AXIS) - centerPoint;
    direction.Normalize();
  }

  const ScalarType deltaAngle = vnl_math::pi / (m_NumberOfSegments / 2);

  int start = 0;
  int end = m_NumberOfSegments;

  if (direction[1] < 0.0)
  {
    direction[0] = -direction[0];
    end = m_NumberOfSegments / 2;
    start = -end;
  }

  vnl_matrix_fixed<mitk::ScalarType, 2, 2> rotation;
  rotation[1][0] = std::sin(std::acos(direction[0]));
  rotation[0][0] = direction[0];
  rotation[1][1] = direction[0];
  rotation[0][1] = -rotation[1][0];

  const ScalarType outerMajorRadius = (m_SizeIsFixed) ? m_FixedRadius : centerPoint.EuclideanDistanceTo(this->GetControlPoint(CP_OUTER_MAJOR_AXIS));
  const ScalarType outerMinorRadius = (m_SizeIsFixed) ? m_FixedRadius : centerPoint.EuclideanDistanceTo(this->GetControlPoint(CP_OUTER_MINOR_AXIS));
  const ScalarType innerMajorRadius = (m_SizeIsFixed) ? (m_FixedRadius-m_FixedThickness) : centerPoint.EuclideanDistanceTo(this->GetControlPoint(CP_INNER_MAJOR_AXIS));
  const ScalarType innerMinorRadius = (m_SizeIsFixed) ? (m_FixedRadius-m_FixedThickness) : innerMajorRadius - (outerMajorRadius - outerMinorRadius);

  ScalarType angle = 0;
  ScalarType cosAngle = 0;
  ScalarType sinAngle = 0;
  vnl_vector_fixed<mitk::ScalarType, 2> vector;
  Point2D point;

  for (int i = start; i < end; ++i)
  {
    angle = i * deltaAngle;
    cosAngle = std::cos(angle);
    sinAngle = std::sin(angle);

    vector[0] = outerMajorRadius * cosAngle;
    vector[1] = outerMinorRadius * sinAngle;
    vector = rotation * vector;

    point[0] = centerPoint[0] + vector[0];
    point[1] = centerPoint[1] + vector[1];

    this->AppendPointToPolyLine(0, point);

    vector[0] = innerMajorRadius * cosAngle;
    vector[1] = innerMinorRadius * sinAngle;
    vector = rotation * vector;

    point[0] = centerPoint[0] + vector[0];
    point[1] = centerPoint[1] + vector[1];

    this->AppendPointToPolyLine(1, point);
  }
}

unsigned int mitk::PlanarDoubleEllipse::GetNumberOfSegments() const
{
  return m_NumberOfSegments;
}

void mitk::PlanarDoubleEllipse::SetNumberOfSegments(unsigned int numSegments)
{
  m_NumberOfSegments = std::max(4U, numSegments);

  if (this->IsPlaced())
  {
    this->GeneratePolyLine();
    this->Modified();
  }
}

unsigned int mitk::PlanarDoubleEllipse::GetMaximumNumberOfControlPoints() const
{
  return (m_SizeIsFixed)? 1 : 4;
}

unsigned int mitk::PlanarDoubleEllipse::GetMinimumNumberOfControlPoints() const
{
  return (m_SizeIsFixed)? 1 : 4;
}

bool mitk::PlanarDoubleEllipse::SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist)
{
  switch (index)
  {
    case 0:
    {
      const Point2D centerPoint = this->GetControlPoint(CP_CENTER);
      const Vector2D vector = point - centerPoint;

      Superclass::SetControlPoint(0, point, createIfDoesNotExist);
      if (!m_SizeIsFixed)
      {
        Superclass::SetControlPoint(1, this->GetControlPoint(CP_OUTER_MAJOR_AXIS) + vector, createIfDoesNotExist);
        Superclass::SetControlPoint(2, this->GetControlPoint(CP_OUTER_MINOR_AXIS) + vector, createIfDoesNotExist);
        Superclass::SetControlPoint(3, this->GetControlPoint(CP_INNER_MAJOR_AXIS) + vector, createIfDoesNotExist);
      }

      break;
    }

    case 1:
    {
      const Vector2D vector = point - this->GetControlPoint(CP_OUTER_MAJOR_AXIS);

      Superclass::SetControlPoint(1, point, createIfDoesNotExist);

      const Point2D centerPoint = this->GetControlPoint(CP_CENTER);
      const Vector2D outerMajorVector = point - centerPoint;

      Vector2D outerMinorVector;
      outerMinorVector[0] = outerMajorVector[1];
      outerMinorVector[1] = -outerMajorVector[0];

      if (!m_ConstrainCircle)
      {
        outerMinorVector.Normalize();
        outerMinorVector *= centerPoint.EuclideanDistanceTo(this->GetControlPoint(CP_OUTER_MINOR_AXIS));
      }

      Superclass::SetControlPoint(2, centerPoint + outerMinorVector, createIfDoesNotExist);

      Vector2D innerMajorVector = outerMajorVector;

      if (!m_ConstrainThickness)
      {
        innerMajorVector.Normalize();
        innerMajorVector *= centerPoint.EuclideanDistanceTo(this->GetControlPoint(CP_INNER_MAJOR_AXIS) - vector);
      }

      Superclass::SetControlPoint(3, centerPoint - innerMajorVector, createIfDoesNotExist);

      break;
    }

    case 2:
    {
      m_ConstrainCircle = false;
      Superclass::SetControlPoint(2, point, createIfDoesNotExist);

      break;
    }

    case 3:
    {
      m_ConstrainThickness = false;
      Superclass::SetControlPoint(3, point, createIfDoesNotExist);

      break;
    }

    default:
      return false;
  }

  return true;
}

bool mitk::PlanarDoubleEllipse::Equals(const mitk::PlanarFigure &other) const
{
  const auto *otherDoubleEllipse = dynamic_cast<const mitk::PlanarDoubleEllipse *>(&other);
  if (otherDoubleEllipse)
  {
    if (this->m_ConstrainCircle != otherDoubleEllipse->m_ConstrainCircle)
      return false;
    if (this->m_ConstrainThickness != otherDoubleEllipse->m_ConstrainThickness)
      return false;
    if (this->m_NumberOfSegments != otherDoubleEllipse->m_NumberOfSegments)
      return false;
    return Superclass::Equals(other);
  }
  else
  {
    return false;
  }
}

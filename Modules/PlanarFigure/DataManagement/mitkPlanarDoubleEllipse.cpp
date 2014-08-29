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

mitk::PlanarDoubleEllipse::~PlanarDoubleEllipse()
{
}

mitk::Point2D mitk::PlanarDoubleEllipse::ApplyControlPointConstraints(unsigned int index, const Point2D& point)
{
  if (index == 2 && !m_ConstrainCircle)
  {
    Point2D centerPoint = this->GetControlPoint(0);
    Vector2D outerMajorVector = this->GetControlPoint(1) - centerPoint;

    Vector2D minorDirection;
    minorDirection[0] = outerMajorVector[1];
    minorDirection[1] = -outerMajorVector[0];
    minorDirection.Normalize();

    double outerMajorRadius = outerMajorVector.GetNorm();
    double innerMajorRadius = (this->GetControlPoint(3) - centerPoint).GetNorm();
    ScalarType radius = std::max(outerMajorRadius - innerMajorRadius, std::min(centerPoint.EuclideanDistanceTo(point), outerMajorRadius));

    return centerPoint + minorDirection * radius;
  }
  else if (index == 3 && !m_ConstrainThickness)
  {
    Point2D centerPoint = this->GetControlPoint(0);
    Vector2D outerMajorVector = this->GetControlPoint(1) - centerPoint;

    double outerMajorRadius = outerMajorVector.GetNorm();
    double outerMinorRadius = (this->GetControlPoint(2) - centerPoint).GetNorm();
    ScalarType radius = std::max(outerMajorRadius - outerMinorRadius, std::min(centerPoint.EuclideanDistanceTo(point), outerMajorRadius));

    outerMajorVector.Normalize();

    return centerPoint - outerMajorVector * radius;
  }

  return point;
}

void mitk::PlanarDoubleEllipse::EvaluateFeaturesInternal()
{
  Point2D centerPoint = this->GetControlPoint(0);
  ScalarType outerMajorRadius = centerPoint.EuclideanDistanceTo(this->GetControlPoint(1));

  this->SetQuantity(FEATURE_ID_MAJOR_AXIS, 2 * outerMajorRadius);
  this->SetQuantity(FEATURE_ID_MINOR_AXIS, 2 * centerPoint.EuclideanDistanceTo(this->GetControlPoint(2)));
  this->SetQuantity(FEATURE_ID_THICKNESS, outerMajorRadius - centerPoint.EuclideanDistanceTo(this->GetControlPoint(3)));
}

void mitk::PlanarDoubleEllipse::GenerateHelperPolyLine(double, unsigned int)
{
}

void mitk::PlanarDoubleEllipse::GeneratePolyLine()
{
  this->ClearPolyLines();

  Point2D centerPoint = this->GetControlPoint(0);
  Point2D outerMajorPoint = this->GetControlPoint(1);

  Vector2D direction = outerMajorPoint - centerPoint;
  direction.Normalize();

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

  ScalarType outerMajorRadius = centerPoint.EuclideanDistanceTo(outerMajorPoint);
  ScalarType outerMinorRadius = centerPoint.EuclideanDistanceTo(this->GetControlPoint(2));
  ScalarType innerMajorRadius = centerPoint.EuclideanDistanceTo(this->GetControlPoint(3));
  ScalarType innerMinorRadius = innerMajorRadius - (outerMajorRadius - outerMinorRadius);

  ScalarType angle;
  ScalarType cosAngle;
  ScalarType sinAngle;
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
  return 4;
}

unsigned int mitk::PlanarDoubleEllipse::GetMinimumNumberOfControlPoints() const
{
  return 4;
}

bool mitk::PlanarDoubleEllipse::SetControlPoint(unsigned int index, const Point2D& point, bool createIfDoesNotExist)
{
  switch (index)
  {
    case 0:
    {
      Point2D centerPoint = this->GetControlPoint(0);
      Vector2D vector = point - centerPoint;

      Superclass::SetControlPoint(0, point, createIfDoesNotExist);
      Superclass::SetControlPoint(1, this->GetControlPoint(1) + vector, createIfDoesNotExist);
      Superclass::SetControlPoint(2, this->GetControlPoint(2) + vector, createIfDoesNotExist);
      Superclass::SetControlPoint(3, this->GetControlPoint(3) + vector, createIfDoesNotExist);

      break;
    }

    case 1:
    {
      Vector2D vector = point - this->GetControlPoint(1);

      Superclass::SetControlPoint(1, point, createIfDoesNotExist);

      Point2D centerPoint = this->GetControlPoint(0);
      Vector2D outerMajorVector = point - centerPoint;

      Vector2D outerMinorVector;
      outerMinorVector[0] = outerMajorVector[1];
      outerMinorVector[1] = -outerMajorVector[0];

      if (!m_ConstrainCircle)
      {
        outerMinorVector.Normalize();
        outerMinorVector *= centerPoint.EuclideanDistanceTo(this->GetControlPoint(2));
      }

      Superclass::SetControlPoint(2, centerPoint + outerMinorVector, createIfDoesNotExist);

      Vector2D innerMajorVector = outerMajorVector;

      if (!m_ConstrainThickness)
      {
        innerMajorVector.Normalize();
        innerMajorVector *= centerPoint.EuclideanDistanceTo(this->GetControlPoint(3) - vector);
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

 bool mitk::PlanarDoubleEllipse::Equals(const mitk::PlanarFigure& other) const
 {
   const mitk::PlanarDoubleEllipse* otherDoubleEllipse = dynamic_cast<const mitk::PlanarDoubleEllipse*>(&other);
   if ( otherDoubleEllipse )
   {
     if( this->m_ConstrainCircle != otherDoubleEllipse->m_ConstrainCircle)
       return false;
     if( this->m_ConstrainThickness != otherDoubleEllipse->m_ConstrainThickness)
       return false;
     if( this->m_NumberOfSegments != otherDoubleEllipse->m_NumberOfSegments)
       return false;
     return Superclass::Equals(other);
   }
   else
   {
     return false;
   }
 }

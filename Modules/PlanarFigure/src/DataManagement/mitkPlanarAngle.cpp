/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarAngle.h"
#include "mitkPlaneGeometry.h"

mitk::PlanarAngle::PlanarAngle() : FEATURE_ID_ANGLE(this->AddFeature("Angle", "deg"))
{
  // Start with two control points
  this->ResetNumberOfControlPoints(2);

  this->SetNumberOfPolyLines(1);
  this->SetNumberOfHelperPolyLines(1);

  m_HelperPolyLinesToBePainted->InsertElement(0, false);
}

void mitk::PlanarAngle::GeneratePolyLine()
{
  this->ClearPolyLines();

  for (unsigned int i = 0; i < this->GetNumberOfControlPoints(); i++)
    this->AppendPointToPolyLine(0, this->GetControlPoint(i));
}

void mitk::PlanarAngle::GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight)
{
  // Generate helper-poly-line for angle
  if (this->GetNumberOfControlPoints() < 3)
  {
    m_HelperPolyLinesToBePainted->SetElement(0, false);
    return; // We do not need to draw an angle as there are no two arms yet
  }

  this->ClearHelperPolyLines();

  const Point2D centerPoint = this->GetControlPoint(1);
  const Point2D boundaryPointOne = this->GetControlPoint(0);
  const Point2D boundaryPointTwo = this->GetControlPoint(2);

  double radius = centerPoint.EuclideanDistanceTo(boundaryPointOne);
  if (radius > centerPoint.EuclideanDistanceTo(boundaryPointTwo))
  {
    radius = centerPoint.EuclideanDistanceTo(boundaryPointTwo);
  }

  // Fixed size radius depending on screen size for the angle
  double nonScalingRadius = displayHeight * mmPerDisplayUnit * 0.05;

  if (nonScalingRadius > radius)
  {
    m_HelperPolyLinesToBePainted->SetElement(0, false);
    return; // if the arc has a radius that is longer than the shortest arm it should not be painted
  }

  m_HelperPolyLinesToBePainted->SetElement(0, true);
  radius = nonScalingRadius;

  double angle = this->GetQuantity(FEATURE_ID_ANGLE);

  // Determine from which arm the angle should be drawn

  Vector2D v0 = boundaryPointOne - centerPoint;
  Vector2D v1 = boundaryPointTwo - centerPoint;
  Vector2D v2;
  v2[0] = 1.0;
  v2[1] = 0.0;

  v0[0] = v0[0] * cos(0.001) - v0[1] * sin(0.001); // rotate one arm a bit
  v0[1] = v0[0] * sin(0.001) + v0[1] * cos(0.001);
  v0.Normalize();
  v1.Normalize();
  double testAngle = acos(v0 * v1);
  // if the rotated arm is closer to the other arm than before it is the one from which we start drawing
  // else we start drawing from the other arm (we want to draw in the mathematically positive direction)
  if (angle > testAngle)
  {
    v1[0] = v0[0] * cos(-0.001) - v0[1] * sin(-0.001);
    v1[1] = v0[0] * sin(-0.001) + v0[1] * cos(-0.001);

    // We determine if the arm is mathematically forward or backward
    // assuming we rotate between -pi and pi
    if (acos(v0 * v2) > acos(v1 * v2))
    {
      testAngle = acos(v1 * v2);
    }
    else
    {
      testAngle = -acos(v1 * v2);
    }
  }
  else
  {
    v0[0] = v1[0] * cos(-0.001) - v1[1] * sin(-0.001);
    v0[1] = v1[0] * sin(-0.001) + v1[1] * cos(-0.001);
    // We determine if the arm is mathematically forward or backward
    // assuming we rotate between -pi and pi
    if (acos(v0 * v2) < acos(v1 * v2))
    {
      testAngle = acos(v1 * v2);
    }
    else
    {
      testAngle = -acos(v1 * v2);
    }
  }
  // Generate poly-line with 16 segments
  for (int t = 0; t < 16; ++t)
  {
    double alpha = (double)t * angle / 15.0 + testAngle;

    Point2D polyLinePoint;
    polyLinePoint[0] = centerPoint[0] + radius * cos(alpha);
    polyLinePoint[1] = centerPoint[1] + radius * sin(alpha);

    this->AppendPointToHelperPolyLine(0, polyLinePoint);
  }
}

void mitk::PlanarAngle::EvaluateFeaturesInternal()
{
  if (this->GetNumberOfControlPoints() < 3)
  {
    // Angle not yet complete.
    return;
  }

  // Calculate angle between lines
  const Point2D &p0 = this->GetControlPoint(0);
  const Point2D &p1 = this->GetControlPoint(1);
  const Point2D &p2 = this->GetControlPoint(2);

  Vector2D v0 = p1 - p0;
  Vector2D v1 = p1 - p2;

  v0.Normalize();
  v1.Normalize();
  double angle = acos(v0 * v1);

  this->SetQuantity(FEATURE_ID_ANGLE, angle);
}

void mitk::PlanarAngle::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

bool mitk::PlanarAngle::Equals(const PlanarFigure &other) const
{
  const auto *otherAngle = dynamic_cast<const mitk::PlanarAngle *>(&other);
  if (otherAngle)
  {
    return Superclass::Equals(other);
  }
  else
  {
    return false;
  }
}

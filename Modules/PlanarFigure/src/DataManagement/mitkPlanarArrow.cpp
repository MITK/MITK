/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarArrow.h"
#include "mitkPlaneGeometry.h"

mitk::PlanarArrow::PlanarArrow() : FEATURE_ID_LENGTH(this->AddFeature("Length", "mm"))
{
  // Directed arrow has two control points
  this->ResetNumberOfControlPoints(2);
  m_ArrowTipScaleFactor = -1.0;

  this->SetNumberOfPolyLines(1);
  this->SetNumberOfHelperPolyLines(2);

  // Create helper polyline object (for drawing the orthogonal orientation line)
  m_HelperPolyLinesToBePainted->InsertElement(0, false);
  m_HelperPolyLinesToBePainted->InsertElement(1, false);
}

void mitk::PlanarArrow::GeneratePolyLine()
{
  this->ClearPolyLines();

  this->AppendPointToPolyLine(0, this->GetControlPoint(0));
  this->AppendPointToPolyLine(0, this->GetControlPoint(1));
}

void mitk::PlanarArrow::GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight)
{
  // Generate helper polyline (orientation line orthogonal to first line)
  // if the third control point is currently being set
  if (this->GetNumberOfControlPoints() != 2)
  {
    m_HelperPolyLinesToBePainted->SetElement(0, false);
    m_HelperPolyLinesToBePainted->SetElement(1, false);

    return;
  }

  this->ClearHelperPolyLines();

  m_HelperPolyLinesToBePainted->SetElement(0, true);
  m_HelperPolyLinesToBePainted->SetElement(1, true);

  // Fixed size depending on screen size for the angle
  float scaleFactor = 0.015;
  if (m_ArrowTipScaleFactor > 0.0)
  {
    scaleFactor = m_ArrowTipScaleFactor;
  }
  double nonScalingLength = displayHeight * mmPerDisplayUnit * scaleFactor;

  // Calculate arrow peak
  const Point2D p1 = this->GetControlPoint(0);
  const Point2D p2 = this->GetControlPoint(1);

  Vector2D n1 = p1 - p2;
  n1.Normalize();

  double degrees = 100.0;
  Vector2D temp;
  temp[0] = n1[0] * cos(degrees) - n1[1] * sin(degrees);
  temp[1] = n1[0] * sin(degrees) + n1[1] * cos(degrees);
  Vector2D temp2;
  temp2[0] = n1[0] * cos(-degrees) - n1[1] * sin(-degrees);
  temp2[1] = n1[0] * sin(-degrees) + n1[1] * cos(-degrees);

  this->AppendPointToHelperPolyLine(0, p1);
  this->AppendPointToHelperPolyLine(0, Point2D(p1 - temp * nonScalingLength));
  this->AppendPointToHelperPolyLine(1, p1);
  this->AppendPointToHelperPolyLine(1, Point2D(p1 - temp2 * nonScalingLength));
}

void mitk::PlanarArrow::EvaluateFeaturesInternal()
{
  // Calculate line length
  const Point3D &p0 = this->GetWorldControlPoint(0);
  const Point3D &p1 = this->GetWorldControlPoint(1);
  double length = p0.EuclideanDistanceTo(p1);

  this->SetQuantity(FEATURE_ID_LENGTH, length);
}

void mitk::PlanarArrow::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

void mitk::PlanarArrow::SetArrowTipScaleFactor(float scale)
{
  m_ArrowTipScaleFactor = scale;
}

bool mitk::PlanarArrow::Equals(const mitk::PlanarFigure &other) const
{
  const auto *otherArrow = dynamic_cast<const mitk::PlanarArrow *>(&other);
  if (otherArrow)
  {
    if (std::abs(this->m_ArrowTipScaleFactor - otherArrow->m_ArrowTipScaleFactor) > mitk::eps)
      return false;

    return Superclass::Equals(other);
  }
  else
  {
    return false;
  }
}

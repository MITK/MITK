/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarLine.h"
#include "mitkPlaneGeometry.h"

mitk::PlanarLine::PlanarLine() : FEATURE_ID_LENGTH(this->AddFeature("Length", "mm"))
{
  // Line has two control points
  this->ResetNumberOfControlPoints(2);

  this->SetNumberOfPolyLines(1);
}

void mitk::PlanarLine::GeneratePolyLine()
{
  this->ClearPolyLines();

  this->AppendPointToPolyLine(0, this->GetControlPoint(0));
  this->AppendPointToPolyLine(0, this->GetControlPoint(1));
}

void mitk::PlanarLine::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
  // A line does not require a helper object
}

void mitk::PlanarLine::EvaluateFeaturesInternal()
{
  // Calculate line length
  const Point3D &p0 = this->GetWorldControlPoint(0);
  const Point3D &p1 = this->GetWorldControlPoint(1);
  double length = p0.EuclideanDistanceTo(p1);

  this->SetQuantity(FEATURE_ID_LENGTH, length);
}

void mitk::PlanarLine::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

bool mitk::PlanarLine::Equals(const PlanarFigure &other) const
{
  const auto *otherLine = dynamic_cast<const mitk::PlanarLine *>(&other);
  if (otherLine)
  {
    return Superclass::Equals(other);
  }
  else
  {
    return false;
  }
}

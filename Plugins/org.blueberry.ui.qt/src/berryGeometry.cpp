/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryGeometry.h"
#include "berryConstants.h"

#include <QPoint>
#include <QRect>
#include <QWidget>

#include <limits>

namespace berry
{

int Geometry::GetDimension(const QRect& toMeasure, bool width)
{
  if (width)
  {
    return toMeasure.width();
  }
  return toMeasure.height();
}

bool Geometry::IsHorizontal(int berrySideConstant)
{
  return !(berrySideConstant == Constants::LEFT || berrySideConstant
      == Constants::RIGHT);
}

QRect Geometry::GetExtrudedEdge(const QRect& toExtrude, int size,
    int orientation)
{
  QRect bounds(toExtrude);

  if (!IsHorizontal(orientation))
  {
    bounds.setWidth(size);
  }
  else
  {
    bounds.setHeight(size);
  }

  if (orientation == Constants::RIGHT)
  {
    bounds.moveLeft(toExtrude.x() + toExtrude.width() - bounds.width());
  }
  else if (orientation == Constants::BOTTOM)
  {
    bounds.moveTop(toExtrude.y() + toExtrude.height() - bounds.height());

  }

  Normalize(bounds);

  return bounds;
}

void Geometry::Normalize(QRect& rect)
{
  if (rect.width() < 0)
  {
    rect.setWidth(-rect.width());
    rect.setX(-rect.width());
  }

  if (rect.height() < 0)
  {
    rect.setHeight(-rect.height());
    rect.setY(-rect.height());
  }
}

int Geometry::GetClosestSide(const QRect& boundary, const QPoint& toTest)
{
  int sides[] =
  { Constants::LEFT, Constants::RIGHT, Constants::TOP, Constants::BOTTOM };

  int closestSide = Constants::LEFT;
  int closestDistance = std::numeric_limits<int>::max();

  for (auto side : sides)
  {


    int distance = GetDistanceFromEdge(boundary, toTest, side);

    if (distance < closestDistance)
    {
      closestDistance = distance;
      closestSide = side;
    }
  }

  return closestSide;
}

int Geometry::GetDistanceFromEdge(const QRect& rectangle,
    const QPoint& testPoint, int edgeOfInterest)
{
  if (edgeOfInterest == Constants::TOP)
    return testPoint.y() - rectangle.y();
  else if (edgeOfInterest == Constants::BOTTOM)
    return rectangle.y() + rectangle.height() - testPoint.y();
  else if (edgeOfInterest == Constants::LEFT)
    return testPoint.x() - rectangle.x();
  else if (edgeOfInterest == Constants::RIGHT)
    return rectangle.x() + rectangle.width() - testPoint.x();

  return 0;
}

int Geometry::GetOppositeSide(int directionConstant)
{
  if (directionConstant == Constants::TOP)
    return Constants::BOTTOM;
  else if (directionConstant == Constants::BOTTOM)
    return Constants::TOP;
  else if (directionConstant == Constants::LEFT)
    return Constants::RIGHT;
  else if (directionConstant == Constants::RIGHT)
    return Constants::LEFT;

  return directionConstant;
}

QRect Geometry::ToControl(QWidget* coordinateSystem,
    const QRect& toConvert)
{
  return QRect(coordinateSystem->mapFromGlobal(toConvert.topLeft()),
               coordinateSystem->mapFromGlobal(toConvert.bottomRight()));
}

QPoint Geometry::ToControl(QWidget* coordinateSystem, const QPoint& toConvert)
{
  return coordinateSystem->mapFromGlobal(toConvert);
}

QRect Geometry::ToDisplay(QWidget* coordinateSystem,
    const QRect& toConvert)
{
  return QRect(coordinateSystem->mapToGlobal(toConvert.topLeft()),
               coordinateSystem->mapToGlobal(toConvert.bottomRight()));
}

QPoint Geometry::ToDisplay(QWidget* coordinateSystem, const QPoint& toConvert)
{
  return coordinateSystem->mapToGlobal(toConvert);
}

}


/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "tweaklets/berryGuiWidgetsTweaklet.h"

#include "berryGeometry.h"
#include "berryConstants.h"

#include <limits>

namespace berry
{

int Geometry::GetDimension(const Rectangle& toMeasure, bool width)
{
  if (width)
  {
    return toMeasure.width;
  }
  return toMeasure.height;
}

bool Geometry::IsHorizontal(int berrySideConstant)
{
  return !(berrySideConstant == Constants::LEFT || berrySideConstant
      == Constants::RIGHT);
}

Rectangle Geometry::GetExtrudedEdge(const Rectangle& toExtrude, int size,
    int orientation)
{
  Rectangle bounds(toExtrude);

  if (!IsHorizontal(orientation))
  {
    bounds.width = size;
  }
  else
  {
    bounds.height = size;
  }

  if (orientation == Constants::RIGHT)
  {
    bounds.x = toExtrude.x + toExtrude.width - bounds.width;
  }
  else if (orientation == Constants::BOTTOM)
  {
    bounds.y = toExtrude.y + toExtrude.height - bounds.height;

  }

  Normalize(bounds);

  return bounds;
}

void Geometry::Normalize(Rectangle& rect)
{
  if (rect.width < 0)
  {
    rect.width = -rect.width;
    rect.x -= rect.width;
  }

  if (rect.height < 0)
  {
    rect.height = -rect.height;
    rect.y -= rect.height;
  }
}

int Geometry::GetClosestSide(const Rectangle& boundary, const Point& toTest)
{
  int sides[] =
  { Constants::LEFT, Constants::RIGHT, Constants::TOP, Constants::BOTTOM };

  int closestSide = Constants::LEFT;
  int closestDistance = std::numeric_limits<int>::max();

  for (unsigned int idx = 0; idx < 4; idx++)
  {
    int side = sides[idx];

    int distance = GetDistanceFromEdge(boundary, toTest, side);

    if (distance < closestDistance)
    {
      closestDistance = distance;
      closestSide = side;
    }
  }

  return closestSide;
}

int Geometry::GetDistanceFromEdge(const Rectangle& rectangle,
    const Point& testPoint, int edgeOfInterest)
{
  if (edgeOfInterest == Constants::TOP)
    return testPoint.y - rectangle.y;
  else if (edgeOfInterest == Constants::BOTTOM)
    return rectangle.y + rectangle.height - testPoint.y;
  else if (edgeOfInterest == Constants::LEFT)
    return testPoint.x - rectangle.x;
  else if (edgeOfInterest == Constants::RIGHT)
    return rectangle.x + rectangle.width - testPoint.x;

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

Rectangle Geometry::ToControl(void* coordinateSystem,
    const Rectangle& toConvert)
{
  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->ToControl(coordinateSystem,
      toConvert);
}

Point Geometry::ToControl(void* coordinateSystem, const Point& toConvert)
{
  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->ToControl(coordinateSystem,
      toConvert);
}

Rectangle Geometry::ToDisplay(void* coordinateSystem,
    const Rectangle& toConvert)
{
  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->ToDisplay(coordinateSystem,
      toConvert);
}

Point Geometry::ToDisplay(void* coordinateSystem, const Point& toConvert)
{
  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->ToDisplay(coordinateSystem,
      toConvert);
}

}


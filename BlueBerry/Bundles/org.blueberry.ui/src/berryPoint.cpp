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

#include "berryPoint.h"

#include <sstream>

namespace berry
{

Point::Point(int x, int y)
{
  this->x = x;
  this->y = y;
}

bool Point::operator==(const Point& p) const
{
  if (&p == this) return true;
  return (p.x == x) && (p.y == y);
}

int Point::HashCode()
{
  return x ^ y;
}

std::string Point::ToString()
{
  std::stringstream str;
  str << "Point {" << x << ", " << y << "}";
  return str.str();
}

}

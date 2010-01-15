/*=========================================================================

Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cherryRectangle.h"

#include <sstream>

namespace cherry
{

Rectangle::Rectangle()
: x(0), y(0), width(0), height(0)
{

}

Rectangle::Rectangle(int x_, int y_, int w, int h) :
  x(x_), y(y_), width(w), height(h)
{

}

void Rectangle::Add(const Rectangle& rect)
{
  int left = x < rect.x ? x : rect.x;
  int top = y < rect.y ? y : rect.y;
  int lhs = x + width;
  int rhs = rect.x + rect.width;
  int right = lhs > rhs ? lhs : rhs;
  lhs = y + height;
  rhs = rect.y + rect.height;
  int bottom = lhs > rhs ? lhs : rhs;
  x = left;
  y = top;
  width = right - left;
  height = bottom - top;
}

bool Rectangle::Contains(int x, int y) const
{
  return (x >= this->x) && (y >= this->y) && ((x - this->x) < width) && ((y
      - this->y) < height);
}

bool Rectangle::Contains(const Point& point) const
{
  return this->Contains(point.x, point.y);
}

Rectangle& Rectangle::FlipXY()
{
  int tmp = x;
  x = y;
  y = tmp;

  tmp = width;
  width = height;
  height = tmp;

  return *this;
}

bool Rectangle::operator==(const Rectangle& r) const
{
  if (&r == this) return true;
  return (r.x == this->x) && (r.y == this->y) && (r.width == this->width) && (r.height == this->height);
}

int Rectangle::HashCode() const
{
  return x ^ y ^ width ^ height;
}

void Rectangle::Intersect(const Rectangle& rect)
{
  if (this == &rect) return;
  int left = x> rect.x ? x : rect.x;
  int top = y> rect.y ? y : rect.y;
  int lhs = x + width;
  int rhs = rect.x + rect.width;
  int right = lhs < rhs ? lhs : rhs;
  lhs = y + height;
  rhs = rect.y + rect.height;
  int bottom = lhs < rhs ? lhs : rhs;
  x = right < left ? 0 : left;
  y = bottom < top ? 0 : top;
  width = right < left ? 0 : right - left;
  height = bottom < top ? 0 : bottom - top;
}

Rectangle Rectangle::Intersection(const Rectangle& rect) const
{
  if (this == &rect) return Rectangle (x, y, width, height);
  int left = x> rect.x ? x : rect.x;
  int top = y> rect.y ? y : rect.y;
  int lhs = x + width;
  int rhs = rect.x + rect.width;
  int right = lhs < rhs ? lhs : rhs;
  lhs = y + height;
  rhs = rect.y + rect.height;
  int bottom = lhs < rhs ? lhs : rhs;
  return Rectangle (
      right < left ? 0 : left,
      bottom < top ? 0 : top,
      right < left ? 0 : right - left,
      bottom < top ? 0 : bottom - top);
}

bool Rectangle::Intersects(int x, int y, int width, int height) const
{
  return (x < this->x + this->width) && (y < this->y + this->height) &&
  (x + width> this->x) && (y + height> this->y);
}

bool Rectangle::Intersects(const Rectangle& rect) const
{
  return &rect == this || this->Intersects(rect.x, rect.y, rect.width, rect.height);
}

int Rectangle::GetDimension(bool width) const
{
  if (width) {
    return this->width;
  }
  return this->height;
}

bool Rectangle::IsEmpty () const
{
  return (width <= 0) || (height <= 0);
}

std::string Rectangle::ToString () const
{
  std::ostringstream stream;
  stream << "Rectangle {" << x << ", " << y << ", " << width << ", " << height << "}";
  return stream.str(); //$NON-NLS-1$//$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$ //$NON-NLS-5$
}

Rectangle Rectangle::Union(const Rectangle& rect) const
{
  int left = x < rect.x ? x : rect.x;
  int top = y < rect.y ? y : rect.y;
  int lhs = x + width;
  int rhs = rect.x + rect.width;
  int right = lhs> rhs ? lhs : rhs;
  lhs = y + height;
  rhs = rect.y + rect.height;
  int bottom = lhs> rhs ? lhs : rhs;
  return Rectangle(left, top, right - left, bottom - top);
}

}

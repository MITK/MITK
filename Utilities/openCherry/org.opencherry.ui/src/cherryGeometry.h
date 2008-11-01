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


#ifndef CHERRYGEOMETRY_H_
#define CHERRYGEOMETRY_H_

#include "cherryRectangle.h"
#include "cherryPoint.h"

namespace cherry {

struct Geometry
{

  /**
   * Returns the height or width of the given rectangle.
   *
   * @param toMeasure rectangle to measure
   * @param width returns the width if true, and the height if false
   * @return the width or height of the given rectangle
   * @since 3.0
   */
  static int GetDimension(const Rectangle& toMeasure, bool width);

  /**
   * Returns true iff the given side constant corresponds to a horizontal side
   * of a rectangle. That is, returns true for the top and bottom but false for the
   * left and right.
   *
   * @param swtSideConstant one of Constants::TOP, Constants::BOTTOM, Constants::LEFT, or Constants::RIGHT
   * @return true iff the given side is horizontal.
   * @since 3.0
   */
  static bool IsHorizontal(int cherrySideConstant);

  /**
   * Extrudes the given edge inward by the given distance. That is, if one side of the rectangle
   * was sliced off with a given thickness, this returns the rectangle that forms the slice. Note
   * that the returned rectangle will be inside the given rectangle if size > 0.
   *
   * @param toExtrude the rectangle to extrude. The resulting rectangle will share three sides
   * with this rectangle.
   * @param size distance to extrude. A negative size will extrude outwards (that is, the resulting
   * rectangle will overlap the original iff this is positive).
   * @param orientation the side to extrude.  One of Constants::LEFT, Constants::RIGHT, Constants::TOP, or Constants::BOTTOM. The
   * resulting rectangle will always share this side with the original rectangle.
   * @return a rectangle formed by extruding the given side of the rectangle by the given distance.
   */
  static Rectangle GetExtrudedEdge(const Rectangle& toExtrude, int size,
          int orientation);

  /**
   * Normalizes the given rectangle. That is, any rectangle with
   * negative width or height becomes a rectangle with positive
   * width or height that extends to the upper-left of the original
   * rectangle.
   *
   * @param rect rectangle to modify
   */
  static void Normalize(Rectangle& rect);

  /**
   * Returns the edge of the given rectangle is closest to the given
   * point.
   *
   * @param boundary rectangle to test
   * @param toTest point to compare
   * @return one of SWT.LEFT, SWT.RIGHT, SWT.TOP, or SWT.BOTTOM
   *
   * @since 3.0
   */
  static int GetClosestSide(const Rectangle& boundary, const Point& toTest);

  /**
   * Returns the distance of the given point from a particular side of the given rectangle.
   * Returns negative values for points outside the rectangle.
   *
   * @param rectangle a bounding rectangle
   * @param testPoint a point to test
   * @param edgeOfInterest side of the rectangle to test against
   * @return the distance of the given point from the given edge of the rectangle
   * @since 3.0
   */
  static int GetDistanceFromEdge(const Rectangle& rectangle, const Point& testPoint,
          int edgeOfInterest);

  /**
   * Returns the opposite of the given direction. That is, returns SWT.LEFT if
   * given SWT.RIGHT and visa-versa.
   *
   * @param swtDirectionConstant one of SWT.LEFT, SWT.RIGHT, SWT.TOP, or SWT.BOTTOM
   * @return one of SWT.LEFT, SWT.RIGHT, SWT.TOP, or SWT.BOTTOM
   * @since 3.0
   */
  static int GetOppositeSide(int directionConstant);

  /**
   * Converts the given rectangle from display coordinates to the local coordinate system
   * of the given object
   *
   * @param coordinateSystem local coordinate system (widget) being converted to
   * @param toConvert rectangle to convert
   * @return a rectangle in control coordinates
   * @since 3.0
   */
  static Rectangle ToControl(void* coordinateSystem,
          const Rectangle& toConvert);

  /**
   * Converts the given point from display coordinates to the local coordinate system
   * of the given object
   *
   * @param coordinateSystem local coordinate system (widget) being converted to
   * @param toConvert point to convert
   * @return a point in control coordinates
   * @since 3.0
   */
  static Point ToControl(void* coordinateSystem,
          const Point& toConvert);

  /**
   * Converts the given rectangle from the local coordinate system of the given object
   * into display coordinates.
   *
   * @param coordinateSystem local coordinate system (widget) being converted from
   * @param toConvert rectangle to convert
   * @return a rectangle in display coordinates
   * @since 3.0
   */
  static Rectangle ToDisplay(void* coordinateSystem,
          const Rectangle& toConvert);

  /**
   * Converts the given point from the local coordinate system of the given object
   * into display coordinates.
   *
   * @param coordinateSystem local coordinate system (widget) being converted from
   * @param toConvert point to convert
   * @return a point in display coordinates
   * @since 3.0
   */
  static Point ToDisplay(void* coordinateSystem,
          const Point& toConvert);

};

}

#endif /* CHERRYGEOMETRY_H_ */

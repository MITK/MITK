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

#ifndef CHERRYRECTANGLE_H_
#define CHERRYRECTANGLE_H_

#include "cherryUiDll.h"
#include "cherryPoint.h"

#include <string>

namespace cherry
{

/**
 * Instances of this class represent rectangular areas in an
 * (x, y) coordinate system. The top left corner of the rectangle
 * is specified by its x and y values, and the extent of the
 * rectangle is specified by its width and height.
 * <p>
 * The coordinate space for rectangles and points is considered
 * to have increasing values downward and to the right from its
 * origin making this the normal, computer graphics oriented notion
 * of (x, y) coordinates rather than the strict mathematical one.
 * </p>
 * <p>
 * The hashCode() method in this class uses the values of the
 * fields to compute the hash value. When storing instances of the
 * class in hashed collections, do not modify these fields after the
 * object has been inserted.
 * </p>
 * <p>
 * Application code does <em>not</em> need to explicitly release the
 * resources managed by each instance when those instances are no longer
 * required, and thus no <code>dispose()</code> method is provided.
 * </p>
 *
 */

struct CHERRY_UI Rectangle
{

  /**
   * the x coordinate of the rectangle
   */
  int x;

  /**
   * the y coordinate of the rectangle
   */
  int y;

  /**
   * the width of the rectangle
   */
  int width;

  /**
   * the height of the rectangle
   */
  int height;

  /**
   * Constrcut a new instance of this class with
   * x, y, width and height values set to zero.
   */
  Rectangle();

  /**
   * Construct a new instance of this class given the
   * x, y, width and height values.
   *
   * @param x the x coordinate of the origin of the rectangle
   * @param y the y coordinate of the origin of the rectangle
   * @param width the width of the rectangle
   * @param height the height of the rectangle
   */
  Rectangle(int x, int y, int width, int height);

  /**
   * Destructively replaces the x, y, width and height values
   * in the receiver with ones which represent the union of the
   * rectangles specified by the receiver and the given rectangle.
   * <p>
   * The union of two rectangles is the smallest single rectangle
   * that completely covers both of the areas covered by the two
   * given rectangles.
   * </p>
   *
   * @param rect the rectangle to merge with the receiver
   *
   * @exception IllegalArgumentException <ul>
   *    <li>ERROR_NULL_ARGUMENT - if the argument is null</li>
   * </ul>
   */
  void Add(const Rectangle& rect);

  /**
   * Returns <code>true</code> if the point specified by the
   * arguments is inside the area specified by the receiver,
   * and <code>false</code> otherwise.
   *
   * @param x the x coordinate of the point to test for containment
   * @param y the y coordinate of the point to test for containment
   * @return <code>true</code> if the rectangle contains the point and <code>false</code> otherwise
   */
  bool Contains(int x, int y) const;

  /**
   * Returns <code>true</code> if the point specified by the
   * arguments is inside the area specified by the receiver,
   * and <code>false</code> otherwise.
   *
   * @param the point to test for containment
   * @return <code>true</code> if the rectangle contains the point and <code>false</code> otherwise
   */
  bool Contains(const Point& point) const;

  /**
   * Flips the x and y coordinates, i.e. rotates the rectangle about 90 degrees.
   *
   * @return the flipped rectangle
   */
  Rectangle& FlipXY();

  /**
   * Compares the argument to the receiver, and returns true
   * if they represent the <em>same</em> object using a class
   * specific comparison.
   *
   * @param object the object to compare with this object
   * @return <code>true</code> if the object is the same as this object and <code>false</code> otherwise
   *
   * @see #hashCode()
   */
  bool operator==(const Rectangle& object) const;

  /**
   * Returns an integer hash code for the receiver. Any two
   * objects that return <code>true</code> when passed to
   * <code>equals</code> must return the same value for this
   * method.
   *
   * @return the receiver's hash
   *
   * @see #equals(Object)
   */
  int HashCode() const;

  /**
   * Destructively replaces the x, y, width and height values
   * in the receiver with ones which represent the intersection of the
   * rectangles specified by the receiver and the given rectangle.
   *
   * @param rect the rectangle to intersect with the receiver
   *
   * @exception IllegalArgumentException <ul>
   *    <li>ERROR_NULL_ARGUMENT - if the argument is null</li>
   * </ul>
   *
   * since 3.0
   */
  void Intersect(const Rectangle& rect);

  /**
   * Returns a new rectangle which represents the intersection
   * of the receiver and the given rectangle.
   * <p>
   * The intersection of two rectangles is the rectangle that
   * covers the area which is contained within both rectangles.
   * </p>
   *
   * @param rect the rectangle to intersect with the receiver
   * @return the intersection of the receiver and the argument
   *
   * @exception IllegalArgumentException <ul>
   *    <li>ERROR_NULL_ARGUMENT - if the argument is null</li>
   * </ul>
   */
  Rectangle Intersection(const Rectangle& rect) const;

  /**
   * Returns <code>true</code> if the rectangle described by the
   * arguments intersects with the receiver and <code>false</code>
   * otherwise.
   * <p>
   * Two rectangles intersect if the area of the rectangle
   * representing their intersection is not empty.
   * </p>
   *
   * @param x the x coordinate of the origin of the rectangle
   * @param y the y coordinate of the origin of the rectangle
   * @param width the width of the rectangle
   * @param height the height of the rectangle
   * @return <code>true</code> if the rectangle intersects with the receiver, and <code>false</code> otherwise
   *
   * @exception IllegalArgumentException <ul>
   *    <li>ERROR_NULL_ARGUMENT - if the argument is null</li>
   * </ul>
   *
   * @see #intersection(Rectangle)
   * @see #isEmpty()
   *
   * @since 3.0
   */
  bool Intersects(int x, int y, int width, int height) const;

  /**
   * Returns <code>true</code> if the given rectangle intersects
   * with the receiver and <code>false</code> otherwise.
   * <p>
   * Two rectangles intersect if the area of the rectangle
   * representing their intersection is not empty.
   * </p>
   *
   * @param rect the rectangle to test for intersection
   * @return <code>true</code> if the rectangle intersects with the receiver, and <code>false</code> otherwise
   *
   * @exception IllegalArgumentException <ul>
   *    <li>ERROR_NULL_ARGUMENT - if the argument is null</li>
   * </ul>
   *
   * @see #intersection(Rectangle)
   * @see #isEmpty()
   */
  bool Intersects(const Rectangle& rect) const;

  /**
   * Returns the height or width
   *
   * @param width returns the width if true, and the height if false
   * @return the width or height of the reciever
   * @since 3.0
   */
  int GetDimension(bool width) const;

  /**
   * Returns <code>true</code> if the receiver does not cover any
   * area in the (x, y) coordinate plane, and <code>false</code> if
   * the receiver does cover some area in the plane.
   * <p>
   * A rectangle is considered to <em>cover area</em> in the
   * (x, y) coordinate plane if both its width and height are
   * non-zero.
   * </p>
   *
   * @return <code>true</code> if the receiver is empty, and <code>false</code> otherwise
   */
  bool IsEmpty() const;

  /**
   * Returns a string containing a concise, human-readable
   * description of the receiver.
   *
   * @return a string representation of the rectangle
   */
  std::string ToString() const;

  /**
   * Returns a new rectangle which represents the union of
   * the receiver and the given rectangle.
   * <p>
   * The union of two rectangles is the smallest single rectangle
   * that completely covers both of the areas covered by the two
   * given rectangles.
   * </p>
   *
   * @param rect the rectangle to perform union with
   * @return the union of the receiver and the argument
   *
   * @exception IllegalArgumentException <ul>
   *    <li>ERROR_NULL_ARGUMENT - if the argument is null</li>
   * </ul>
   *
   * @see #add(Rectangle)
   */
  Rectangle Union(const Rectangle& rect) const;

};

}
#endif /*CHERRYRECTANGLE_H_*/

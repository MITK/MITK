/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLine_h
#define mitkLine_h

#include <mitkNumericTypes.h>
#include <itkMatrix.h>
#include <itkTransform.h>
#include <vnl/vnl_cross.h>
#include <vnl/vnl_vector.h>

namespace mitk
{
  /**
   * \brief Describes a line in N-dimensional space.
   *
   * A line is defined by a start point and a direction vector. The length of
   * the direction vector defines the length of the line segment for methods
   * like IsPartOfStraightLine().
   *
   * \ingroup Geometry
   * \tparam TCoordRep The coordinate representation type.
   * \tparam NPointDimension The dimensionality of the space (default is 3).
   */
  template <class TCoordRep, unsigned int NPointDimension = 3>
  class Line
  {
  public:
    Line()
    {
      m_Point.Fill(0);
      m_Direction.Fill(0);
    }

    /** \brief Define line by point and direction.
     *
     * Length of direction defines the length of the line.
     *
     * \param point Start point of the line.
     * \param direction Direction vector of the line.
     */
    Line(const itk::Point<TCoordRep, NPointDimension> &point, const itk::Vector<TCoordRep, NPointDimension> &direction)
    {
      this->m_Point = point;
      this->m_Direction = direction;
    }

    /** \brief Get start point of the line (const). */
    const itk::Point<TCoordRep, NPointDimension> &GetPoint() const { return m_Point; }

    /** \brief Get start point of the line (non-const). */
    itk::Point<TCoordRep, NPointDimension> &GetPoint() { return m_Point; }

    /** \brief Get point on the line at parameter \a t.
     *
     * \param t Parameter along the line direction.
     * \return m_Point + t * m_Direction
     */
    const itk::Point<TCoordRep, NPointDimension> GetPoint(TCoordRep t) const { return m_Point + m_Direction * t; }

    /** \brief Set/change start point of the line.
     *
     * The end point of the line is preserved, so the direction vector is
     * recomputed accordingly.
     *
     * \param point1 New start point.
     */
    void SetPoint(const itk::Point<TCoordRep, NPointDimension> &point1)
    {
      itk::Point<TCoordRep, NPointDimension> point2;
      point2 = m_Point + m_Direction;
      m_Point = point1;
      m_Direction = point2.GetVectorFromOrigin() - point1.GetVectorFromOrigin();
    }

    /** \brief Get the direction vector of the line (const). */
    const itk::Vector<TCoordRep, NPointDimension> &GetDirection() const { return m_Direction; }

    /** \brief Get the direction vector of the line (non-const). */
    itk::Vector<TCoordRep, NPointDimension> &GetDirection() { return m_Direction; }

    /** \brief Set the direction vector of the line.
     *
     * \param direction New direction vector.
     */
    void SetDirection(const itk::Vector<TCoordRep, NPointDimension> &direction) { m_Direction = direction; }

    /** \brief Define line by point and direction.
     *
     * Length of direction defines the length of the line.
     *
     * \param point Start point of the line.
     * \param direction Direction vector of the line.
     */
    void Set(const itk::Point<TCoordRep, NPointDimension> &point,
             const itk::Vector<TCoordRep, NPointDimension> &direction)
    {
      this->m_Point = point;
      this->m_Direction = direction;
    }

    /** \brief Define line by two points.
     *
     * \param point1 Start point of the line.
     * \param point2 End point of the line.
     */
    void SetPoints(const itk::Point<TCoordRep, NPointDimension> &point1,
                   const itk::Point<TCoordRep, NPointDimension> &point2)
    {
      this->m_Point = point1;
      // this->m_Direction.sub( point2, point1 );
      m_Direction = point2 - point1;
    }

    /** \brief Set/change start point of the line.
     *
     * The end point of the line is preserved, so the direction vector is
     * recomputed accordingly.
     *
     * \param point1 New start point.
     */
    void SetPoint1(const itk::Point<TCoordRep, NPointDimension> &point1)
    {
      itk::Vector<TCoordRep, NPointDimension> point2;
      point2 = m_Point.GetVectorFromOrigin() + m_Direction;

      m_Point = point1;
      m_Direction = point2 - point1.GetVectorFromOrigin();
    }

    /** \brief Get start point of the line. */
    const itk::Point<TCoordRep, NPointDimension> &GetPoint1() const { return m_Point; }

    /** \brief Set/change end point of the line.
     *
     * The start point is preserved, so the direction vector is recomputed.
     *
     * \param point2 New end point.
     */
    void SetPoint2(const itk::Point<TCoordRep, NPointDimension> &point2) { m_Direction = point2 - m_Point; }

    /** \brief Get end point of the line.
     *
     * \return The point at m_Point + m_Direction.
     */
    itk::Point<TCoordRep, NPointDimension> GetPoint2() const
    {
      itk::Point<TCoordRep, NPointDimension> point2;
      point2 = m_Point + m_Direction;
      return point2;
    }

    /** \brief Transform the line with an ITK Transform.
     *
     * Both the start point and the direction vector are transformed.
     *
     * \param transform The ITK transform to apply.
     */
    void Transform(itk::Transform<TCoordRep, NPointDimension, NPointDimension> &transform)
    {
      m_Direction = transform.TransformVector(m_Direction);
      m_Point = transform.TransformPoint(m_Point);
    }

    /** \brief Transform the line with a matrix.
     *
     * Only the direction will be changed, not the start point.
     *
     * \param matrix The transformation matrix to apply.
     */
    void Transform(const itk::Matrix<TCoordRep, NPointDimension, NPointDimension> &matrix)
    {
      m_Direction = matrix * m_Direction;
    }

    /** \brief Compute the distance of a point from the line.
     *
     * \param point The point to compute the distance for.
     * \return The perpendicular distance from the point to the line.
     */
    double Distance(const itk::Point<TCoordRep, NPointDimension> &point) const
    {
      itk::Vector<TCoordRep, NPointDimension> diff;
      diff = Project(point) - point;
      return diff.GetNorm();
    }

    /** \brief Project a point onto the line.
     *
     * If the direction vector has zero norm, the start point is returned.
     *
     * \param point The point to project.
     * \return The projected point on the line.
     */
    itk::Point<TCoordRep, NPointDimension> Project(const itk::Point<TCoordRep, NPointDimension> &point) const
    {
      if (m_Direction.GetNorm() == 0)
        return this->m_Point;

      itk::Vector<TCoordRep, NPointDimension> diff;
      diff = point - this->m_Point;

      itk::Vector<TCoordRep, NPointDimension> normalizedDirection = m_Direction;
      normalizedDirection.Normalize();

      normalizedDirection *= dot_product(diff.GetVnlVector(), normalizedDirection.GetVnlVector());

      return this->m_Point + normalizedDirection;
    }

    /** \brief Test if a point is part of the line segment.
     *
     * The length of the direction vector defines the length of the line segment.
     * The point must lie on the line and within the segment bounds.
     *
     * \param point The point to test.
     * \return True if the point is on the line segment, false otherwise.
     */
    bool IsPartOfStraightLine(const itk::Point<TCoordRep, NPointDimension> &point) const
    {
      if (Distance(point) > eps)
        return false;

      itk::Vector<TCoordRep, NPointDimension> diff;
      diff = point - this->m_Point;

      if (diff * m_Direction < 0)
        return false;

      if (diff.GetSquaredNorm() <= m_Direction.GetSquaredNorm())
        return true;

      return false;
    }

    /** \brief Test if a point is part of the line (line having infinite length).
     *
     * \param point The point to test.
     * \return True if the point lies on the infinite line, false otherwise.
     */
    bool IsPartOfLine(const itk::Point<TCoordRep, NPointDimension> &point) const
    {
      if (Distance(point) < eps)
        return true;

      return false;
    }

    /** \brief Test if a line is parallel to this line.
     *
     * Uses the cross product of direction vectors (3D only).
     *
     * \param line The other line to compare.
     * \return True if the lines are parallel, false otherwise.
     */
    bool IsParallel(const Line<TCoordRep, NPointDimension> &line) const
    {
      vnl_vector<TCoordRep> normal;

      normal = vnl_cross_3d(m_Direction.GetVnlVector(), line.GetDirection().GetVnlVector());

      if (normal.squared_magnitude() < eps)
        return true;

      return false;
    }

    /** \brief Test if another line is part of this line (line having infinite length).
     *
     * The other line must be parallel and share a common point.
     *
     * \param line The other line to test.
     * \return True if the other line lies on this infinite line, false otherwise.
     */
    bool IsPartOfLine(const Line<TCoordRep, NPointDimension> &line) const
    {
      return (Distance(line.GetPoint()) < 0) && (IsParallel(line));
    }

    /** \brief Test if the two lines are identical.
     *
     * Start point and direction and length of direction vector must be
     * equal for identical lines.
     *
     * \param line The other line to compare.
     * \return True if both lines are identical, false otherwise.
     */
    bool operator==(const Line<TCoordRep, NPointDimension> &line) const
    {
      itk::Vector<TCoordRep, NPointDimension> diff;
      diff = GetPoint1() - line.GetPoint1();
      if (diff.GetSquaredNorm() > eps)
        return false;
      diff = GetPoint2() - line.GetPoint2();
      if (diff.GetSquaredNorm() > eps)
        return false;
      return true;
    }

    /** \brief Set the line by another line (copy assignment).
     *
     * \param line The line to copy from.
     * \return Reference to this line.
     */
    inline const Line<TCoordRep, NPointDimension> &operator=(const Line<TCoordRep, NPointDimension> &line)
    {
      m_Point = line.GetPoint();
      m_Direction = line.GetDirection();
      return *this;
    }

    /** \brief Test if two lines are not identical.
     *
     * \sa operator==
     */
    bool operator!=(const Line<TCoordRep, NPointDimension> &line) const { return !((*this) == line); }

    /** \brief Calculates the intersection points of a straight line in 2D
     * with a rectangle.
     *
     * \param x1 Left boundary of the rectangle.
     * \param y1 Bottom boundary of the rectangle.
     * \param x2 Right boundary of the rectangle.
     * \param y2 Top boundary of the rectangle.
     * \param p A point on the straight line.
     * \param d Direction vector of the straight line.
     * \param s1 First intersection point (valid only if return value > 0).
     * \param s2 Second intersection point (valid only if return value == 2).
     * \return Number of intersection points (0 <= return value <= 2).
     */
    static int RectangleLineIntersection(TCoordRep x1,
                                         TCoordRep y1,
                                         TCoordRep x2,
                                         TCoordRep y2,
                                         itk::Point<TCoordRep, 2> p,
                                         itk::Vector<TCoordRep, 2> d,
                                         itk::Point<TCoordRep, 2> &s1,
                                         itk::Point<TCoordRep, 2> &s2)
    {
      int s_num;
      TCoordRep t;
      s_num = 0;

      /*test if intersecting with the horizontal axis*/
      if (fabs(d[0]) > eps)
      {
        t = (x1 - p[0]) / d[0];
        itk::Point<TCoordRep, 2> l = p + d * t;
        if ((l[1] >= y1) && (l[1] < y2))
        { // yes, intersection point within the bounds of the border-line
          if (s_num)
            s2 = l;
          else
            s1 = l;
          ++s_num;
        }
      }

      if (fabs(d[0]) > eps)
      {
        t = (x2 - p[0]) / d[0];
        itk::Point<TCoordRep, 2> l = p + d * t;

        if ((l[1] >= y1) && (l[1] < y2))
        { // yes, intersection point within the bounds of the border-line
          if (s_num)
            s2 = l;
          else
            s1 = l;
          ++s_num;
        }
      }

      /*test if intersecting with the vertical axis*/
      if (fabs(d[1]) > eps)
      {
        t = (y1 - p[1]) / d[1];
        itk::Point<TCoordRep, 2> l = p + d * t;

        if ((l[0] >= x1) && (l[0] < x2))
        { // yes, intersection point within the bounds of the border-line
          if (s_num)
            s2 = l;
          else
            s1 = l;
          ++s_num;
        }
      }

      if (fabs(d[1]) > eps)
      {
        t = (y2 - p[1]) / d[1];
        itk::Point<TCoordRep, 2> l = p + d * t;
        if ((l[0] >= x1) && (l[0] < x2))
        { // yes, intersection point within the bounds of the border-line
          if (s_num)
            s2 = l;
          else
            s1 = l;
          ++s_num;
        }
      }
      return s_num;
    }

    /**
     * \brief Calculates the intersection points of a straight line in 3D with
     * a box.
     *
     * \param x1,y1,z1  first corner of the box
     * \param x2,y2,z2  second corner of the box
     * \param p,d       straight line: p point on it, d direction of line
     * \param s1        first intersection point (valid only if s_num>0)
     * \param s2        second intersection point (valid only if s_num==2)
     * \return          number of intersection points (0<=s_num<=2)
     */
    static int BoxLineIntersection(TCoordRep x1,
                                   TCoordRep y1,
                                   TCoordRep z1,
                                   TCoordRep x2,
                                   TCoordRep y2,
                                   TCoordRep z2,
                                   itk::Point<TCoordRep, 3> p,
                                   itk::Vector<TCoordRep, 3> d,
                                   itk::Point<TCoordRep, 3> &s1,
                                   itk::Point<TCoordRep, 3> &s2)
    {
      int num = 0;

      ScalarType box[6];
      box[0] = x1;
      box[1] = x2;
      box[2] = y1;
      box[3] = y2;
      box[4] = z1;
      box[5] = z2;

      itk::Point<TCoordRep, 3> point;

      int i, j;
      for (i = 0; i < 6; ++i)
      {
        j = i / 2;
        if (fabs(d[j]) > eps)
        {
          ScalarType lambda = (box[i] - p[j]) / d[j];

          point = p + d * lambda;

          int k = (j + 1) % 3;
          int l = (j + 2) % 3;

          if ((((point[k] >= box[k * 2]) && (point[k] <= box[k * 2 + 1])) ||
               ((point[k] <= box[k * 2]) && (point[k] >= box[k * 2 + 1]))) &&
              (((point[l] >= box[l * 2]) && (point[l] <= box[l * 2 + 1])) ||
               ((point[l] <= box[l * 2]) && (point[l] >= box[l * 2 + 1])))

                )
          {
            if (num == 0)
            {
              s1 = point;
            }
            else
            {
              s2 = point;
            }
            ++num;
          }
        }
      }
      return num;
    }

  protected:
    itk::Point<TCoordRep, NPointDimension> m_Point;
    itk::Vector<TCoordRep, NPointDimension> m_Direction;
  };

  typedef Line<ScalarType, 3> Line3D;

} // namespace mitk

#endif

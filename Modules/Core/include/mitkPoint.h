/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPoint_h
#define mitkPoint_h

#include <itkPoint.h>

#include <mitkArray.h>
#include <mitkEqual.h>
#include <mitkNumericConstants.h>

#include <nlohmann/json.hpp>

namespace mitk
{
  /** \brief Enumeration of point specification types.
   *
   * Describes the role of a point within a geometric structure (e.g. start, corner, edge, end).
   */
  enum PointSpecificationType
  {
    PTUNDEFINED = 0,
    PTSTART,
    PTCORNER,
    PTEDGE,
    PTEND
  };

  /** \brief MITK point type extending itk::Point with additional constructors and utility methods.
   *
   * Provides constructors for various input types and utility methods for
   * copying to/from array types.
   *
   * \tparam TCoordRep Coordinate representation type (e.g. double, float).
   * \tparam NPointDimension Number of dimensions (default: 3).
   */
  template <class TCoordRep, unsigned int NPointDimension = 3>
  class Point : public itk::Point<TCoordRep, NPointDimension>
  {
  public:
    /** \brief Default constructor. */
    explicit Point() : itk::Point<TCoordRep, NPointDimension>() {}
    /** \brief Pass-through constructors for the Array base class. */
    template <typename TPointValueType>
    explicit Point(const Point<TPointValueType, NPointDimension> &r) : itk::Point<TCoordRep, NPointDimension>(r)
    {
    }

    template <typename TPointValueType>
    explicit Point(const TPointValueType r[NPointDimension]) : itk::Point<TCoordRep, NPointDimension>(r)
    {
    }

    template <typename TPointValueType>
    explicit Point(const TPointValueType &v) : itk::Point<TCoordRep, NPointDimension>(v)
    {
    }

    Point(const mitk::Point<TCoordRep, NPointDimension> &r)
      : itk::Point<TCoordRep, NPointDimension>(r)
    {
    }
    Point(const TCoordRep r[NPointDimension]) : itk::Point<TCoordRep, NPointDimension>(r) {}
    Point(const TCoordRep &v) : itk::Point<TCoordRep, NPointDimension>(v) {}
    Point(const itk::Point<TCoordRep, NPointDimension> &p)
      : itk::Point<TCoordRep, NPointDimension>(p)
    {
    }

    template <typename... Args,
              typename = std::enable_if_t<(sizeof...(Args) == NPointDimension)>>
    explicit Point(Args... args)
    {
      size_t i = 0;
      ((this->GetDataPointer()[i++] = static_cast<TCoordRep>(args)), ...);
    }

    /**
     * \brief Copy elements from an array into this point.
     *
     * Note that this method will assign doubles to floats without complaining!
     *
     * \param[in] array The array whose values shall be copied. Must overload [] operator.
     */
    template <typename ArrayType>
    void FillPoint(const ArrayType &array)
    {
      itk::FixedArray<TCoordRep, NPointDimension> *thisP =
        dynamic_cast<itk::FixedArray<TCoordRep, NPointDimension> *>(this);
      mitk::FillArray<ArrayType, TCoordRep, NPointDimension>(*thisP, array);
    }

    /**
     * \brief Copy the values stored in this point into an array.
     *
     * \param[out] array The array which should store the values of this point.
     */
    template <typename ArrayType>
    void ToArray(ArrayType array) const
    {
      mitk::ToArray<ArrayType, TCoordRep, NPointDimension>(array, *this);
    }
  };

  /** \brief Serialize a mitk::Point to a JSON array. */
  template <class TCoordRep, unsigned int NPointDimension>
  void to_json(nlohmann::json& j, const Point<TCoordRep, NPointDimension>& p)
  {
    j = nlohmann::json::array();

    for (size_t i = 0; i < NPointDimension; ++i)
      j.push_back(p[i]);
  }

  /** \brief Deserialize a mitk::Point from a JSON array. */
  template <class TCoordRep, unsigned int NPointDimension>
  void from_json(const nlohmann::json& j, Point<TCoordRep, NPointDimension>& p)
  {
    for (size_t i = 0; i < NPointDimension; ++i)
      j.at(i).get_to(p[i]);
  }

  typedef Point<ScalarType, 2> Point2D;
  typedef Point<ScalarType, 3> Point3D;
  typedef Point<ScalarType, 4> Point4D;

  typedef Point<int, 2> Point2I;
  typedef Point<int, 3> Point3I;
  typedef Point<int, 4> Point4I;

  /**
   * \ingroup MITKTestingAPI
   * \brief Compare two itk::Point instances for equality within a tolerance.
   *
   * \param[in] point1 Point to compare.
   * \param[in] point2 Point to compare.
   * \param[in] eps Tolerance for floating point comparison.
   * \param[in] verbose Flag indicating detailed console output.
   * \return True if points are equal within the given tolerance.
   */
  template <typename TCoordRep, unsigned int NPointDimension>
  inline bool Equal(const itk::Point<TCoordRep, NPointDimension> &point1,
                    const itk::Point<TCoordRep, NPointDimension> &point2,
                    TCoordRep eps = mitk::eps,
                    bool verbose = false)
  {
    bool isEqual = true;
    typename itk::Point<TCoordRep, NPointDimension>::VectorType diff = point1 - point2;
    for (unsigned int i = 0; i < NPointDimension; i++)
    {
      if (DifferenceBiggerOrEqualEps(diff[i], eps))
      {
        isEqual = false;
        break;
      }
    }

    ConditionalOutputOfDifference(point1, point2, eps, verbose, isEqual);

    return isEqual;
  }

} // namespace mitk

#endif

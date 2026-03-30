/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVector_h
#define mitkVector_h

#include <itkVector.h>
#include <vnl/vnl_vector.h>
#include <vnl/vnl_vector_fixed.h>

#include <mitkArray.h>
#include <mitkEqual.h>
#include <mitkExceptionMacro.h>
#include <mitkNumericConstants.h>

#include <nlohmann/json.hpp>

namespace mitk
{
  /** \brief MITK vector type extending itk::Vector with additional constructors and conversions.
   *
   * Provides constructors from vnl_vector, vnl_vector_fixed, and variadic arguments,
   * as well as conversion operators and utility methods for copying to/from array types.
   *
   * \tparam TCoordRep Coordinate representation type (e.g. double, float).
   * \tparam NVectorDimension Number of dimensions (default: 3).
   */
  template <class TCoordRep, unsigned int NVectorDimension = 3>
  class Vector : public itk::Vector<TCoordRep, NVectorDimension>
  {
  public:
    /** \brief Default constructor. */
    explicit Vector() : itk::Vector<TCoordRep, NVectorDimension>() {}

    /** \brief Copy constructor. */
    explicit Vector(const mitk::Vector<TCoordRep, NVectorDimension> &r)
      : itk::Vector<TCoordRep, NVectorDimension>(r)
    {
    }

    /** Pass-through assignment operator for the Vector base class. */
    Vector<TCoordRep, NVectorDimension> & operator=(const Vector<TCoordRep, NVectorDimension> & r)
    {
      itk::Vector<TCoordRep, NVectorDimension>::operator=(r);
      return *this;
    }

    /**
     * \brief Constructor to convert from itk::Vector to mitk::Vector.
     */
    Vector(const itk::Vector<TCoordRep, NVectorDimension> &r)
      : itk::Vector<TCoordRep, NVectorDimension>(r)
    {
    }

    /**
     * \brief Constructor to convert an array to mitk::Vector.
     * \param[in] r The array.
     * \attention Must have NVectorDimension valid arguments!
     */
    Vector(const TCoordRep r[NVectorDimension])
      : itk::Vector<TCoordRep, NVectorDimension>(r)
    {
    }

    /**
     * Constructor to initialize entire vector to one value.
     */
    Vector(const TCoordRep &v) : itk::Vector<TCoordRep, NVectorDimension>(v) {}
    /**
     * \brief Constructor for vnl_vectors.
     * \throws mitk::Exception If vnl_vector.size() != NVectorDimension.
     */
    Vector(const vnl_vector<TCoordRep> &vnlVector)
      : itk::Vector<TCoordRep, NVectorDimension>()
    {
      if (vnlVector.size() != NVectorDimension)
        mitkThrow() << "when constructing mitk::Vector from vnl_vector: sizes didn't match: mitk::Vector "
                    << NVectorDimension << "; vnl_vector " << vnlVector.size();

      for (unsigned int var = 0; (var < NVectorDimension) && (var < vnlVector.size()); ++var)
      {
        this->SetElement(var, vnlVector.get(var));
      }
    }

    /**
     * \brief Constructor for vnl_vector_fixed.
     */
    Vector(const vnl_vector_fixed<TCoordRep, NVectorDimension> &vnlVectorFixed)
      : itk::Vector<TCoordRep, NVectorDimension>()
    {
      for (unsigned int var = 0; var < NVectorDimension; ++var)
      {
        this->SetElement(var, vnlVectorFixed[var]);
      }
    }

    template <typename... Args,
              typename = std::enable_if_t<(sizeof...(Args) == NVectorDimension)>>
    explicit Vector(Args... args)
    {
      size_t i = 0;
      ((this->GetDataPointer()[i++] = static_cast<TCoordRep>(args)), ...);
    }

    /**
     * \brief Copy elements from an array into this vector.
     *
     * Note that this method will assign doubles to floats without complaining!
     *
     * \param[in] array The array whose values shall be copied. Must overload [] operator.
     */
    template <typename ArrayType>
    void FillVector(const ArrayType &array)
    {
      itk::FixedArray<TCoordRep, NVectorDimension> *thisP =
        dynamic_cast<itk::FixedArray<TCoordRep, NVectorDimension> *>(this);
      mitk::FillArray<ArrayType, TCoordRep, NVectorDimension>(*thisP, array);
    }

    /**
     * \brief Copy the values stored in this vector into an array.
     *
     * \param[out] array The array which should store the values of this vector.
     */
    template <typename ArrayType>
    void ToArray(ArrayType array) const
    {
      mitk::ToArray<ArrayType, TCoordRep, NVectorDimension>(array, *this);
    }

    /**
     * \brief User-defined conversion of mitk::Vector to vnl_vector.
     *
     * Note: the conversion to vnl_vector_fixed has not been implemented since this
     * would collide with the conversion vnl_vector to vnl_vector_fixed provided by vnl.
     */
    operator vnl_vector<TCoordRep>() const { return this->GetVnlVector(); }
  }; // end mitk::Vector

  /** \brief Serialize a mitk::Vector to a JSON array. */
  template <class TCoordRep, unsigned int NVectorDimension>
  void to_json(nlohmann::json &j, const Vector<TCoordRep, NVectorDimension> &v)
  {
    j = nlohmann::json::array();

    for (size_t i = 0; i < NVectorDimension; ++i)
      j.push_back(v[i]);
  }

  /** \brief Deserialize a mitk::Vector from a JSON array. */
  template <class TCoordRep, unsigned int NVectorDimension>
  void from_json(const nlohmann::json &j, Vector<TCoordRep, NVectorDimension> &v)
  {
    for (size_t i = 0; i < NVectorDimension; ++i)
      j.at(i).get_to(v[i]);
  }

  // convenience typedefs for often used mitk::Vector representations.

  typedef Vector<ScalarType, 2> Vector2D;
  typedef Vector<ScalarType, 3> Vector3D;
  typedef Vector<ScalarType, 4> Vector4D;

  // other vector types used in MITK
  typedef vnl_vector<ScalarType> VnlVector;

  // The equal methods to compare vectors for equality are below:

  /**
   * \ingroup MITKTestingAPI
   * \brief Compare two itk::Vector instances for equality within a tolerance.
   *
   * \param[in] vector1 Vector to compare.
   * \param[in] vector2 Vector to compare.
   * \param[in] eps Tolerance for floating point comparison.
   * \param[in] verbose Flag indicating detailed console output.
   * \return True if vectors are equal within the given tolerance.
   */
  template <typename TCoordRep, unsigned int NPointDimension>
  inline bool Equal(const itk::Vector<TCoordRep, NPointDimension> &vector1,
                    const itk::Vector<TCoordRep, NPointDimension> &vector2,
                    TCoordRep eps = mitk::eps,
                    bool verbose = false)
  {
    bool isEqual = true;
    typename itk::Vector<TCoordRep, NPointDimension>::VectorType diff = vector1 - vector2;
    for (unsigned int i = 0; i < NPointDimension; i++)
    {
      if (DifferenceBiggerOrEqualEps(diff[i], eps))
      {
        isEqual = false;
        break;
      }
    }

    ConditionalOutputOfDifference(vector1, vector2, eps, verbose, isEqual);

    return isEqual;
  }

  /**
   * \ingroup MITKTestingAPI
   * \brief Compare two VnlVector instances for equality within a tolerance.
   *
   * \param[in] vector1 Vector to compare.
   * \param[in] vector2 Vector to compare.
   * \param[in] eps Tolerance for floating point comparison.
   * \param[in] verbose Flag indicating detailed console output.
   * \return True if vectors are equal within the given tolerance.
   */
  inline bool Equal(const mitk::VnlVector &vector1,
                    const mitk::VnlVector &vector2,
                    ScalarType eps = mitk::eps,
                    bool verbose = false)
  {
    bool isEqual = true;
    mitk::VnlVector diff = vector1 - vector2;
    for (unsigned int i = 0; i < diff.size(); i++)
    {
      if (DifferenceBiggerOrEqualEps(diff[i], eps))
      {
        isEqual = false;
        break;
      }
    }

    ConditionalOutputOfDifference(vector1, vector2, eps, verbose, isEqual);

    return isEqual;
  }

  /**
   * \ingroup MITKTestingAPI
   * \brief Compare two vnl_vector_fixed instances for equality within a tolerance.
   *
   * \param[in] vector1 Vector to compare.
   * \param[in] vector2 Vector to compare.
   * \param[in] eps Tolerance for floating point comparison.
   * \param[in] verbose Flag indicating detailed console output.
   * \return True if vectors are equal within the given tolerance.
   */
  template <typename TCoordRep, unsigned int NPointDimension>
  inline bool Equal(const vnl_vector_fixed<TCoordRep, NPointDimension> &vector1,
                    const vnl_vector_fixed<TCoordRep, NPointDimension> &vector2,
                    TCoordRep eps = mitk::eps,
                    bool verbose = false)
  {
    vnl_vector_fixed<TCoordRep, NPointDimension> diff = vector1 - vector2;
    bool isEqual = true;
    for (unsigned int i = 0; i < diff.size(); i++)
    {
      if (DifferenceBiggerOrEqualEps(diff[i], eps))
      {
        isEqual = false;
        break;
      }
    }

    ConditionalOutputOfDifference(vector1, vector2, eps, verbose, isEqual);

    return isEqual;
  }

} // end namespace mitk

#endif

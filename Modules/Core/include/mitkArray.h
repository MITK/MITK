/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkArray_h
#define mitkArray_h

#include <itkFixedArray.h>

#include <mitkEqual.h>
#include <mitkNumericConstants.h>

namespace mitk
{
  /**
   * \brief Methods to copy between itk::FixedArray types (like mitk::Vector and mitk::Point) and ArrayTypes.
   *
   * ArrayTypes here are all types that implement operator[].
   * The templated methods are free-floating so you may specialize them for your concrete type.
   */

  /**
   * \brief Copy elements of an array into an itk::FixedArray.
   *
   * \param[out] toArray The FixedArray (e.g., mitk::Vector or mitk::Point) which should hold the elements of array.
   * \param[in] array The array whose values will be copied. Must be of a type which overrides the [] operator.
   * \attention array must be of dimension NVectorDimension!
   * \attention This method implicitly converts between data types.
   */
  template <typename ArrayType, typename TCoordRep, unsigned int NVectorDimension>
  void FillArray(itk::FixedArray<TCoordRep, NVectorDimension> &toArray, const ArrayType &array)
  {
    for (unsigned short int var = 0; var < NVectorDimension; ++var)
    {
      toArray[var] = array[var];
    }
  }

  /**
   * \brief Copy elements of an array into a new itk::FixedArray and return it.
   *
   * \param[in] array The array whose values will be copied. Must be of a type which overrides the [] operator.
   * \return A new FixedArray (e.g., mitk::Vector or mitk::Point) holding the elements of array.
   * \attention array must be of dimension NVectorDimension!
   * \attention This method implicitly converts between data types.
   */
  template <typename ArrayType, typename TCoordRep, unsigned int NVectorDimension>
  itk::FixedArray<TCoordRep, NVectorDimension> FillArray(const ArrayType &array)
  {
    itk::FixedArray<TCoordRep, NVectorDimension> vectorOrPoint;

    mitk::FillArray(vectorOrPoint, array);

    return vectorOrPoint;
  }

  /**
   * \brief Copy elements from an itk::FixedArray into an array.
   *
   * \param[out] array The array which will hold the elements. Must be of a type which overrides the [] operator.
   * \param[in] vectorOrPoint The itk::FixedArray which shall be copied. Can e.g. be mitk::Vector or mitk::Point.
   * \attention array must be of dimension NVectorDimension!
   * \attention This method implicitly converts between data types.
   */
  template <typename ArrayType, typename TCoordRep, unsigned int NVectorDimension>
  void ToArray(ArrayType &array, const itk::FixedArray<TCoordRep, NVectorDimension> &vectorOrPoint)
  {
    for (unsigned short int var = 0; var < NVectorDimension; ++var)
    {
      array[var] = vectorOrPoint[var];
    }
  }

  /**
   * \brief Copy elements from an itk::FixedArray into a new array and return it.
   *
   * \param[in] vectorOrPoint The itk::FixedArray which shall be copied. Can e.g. be mitk::Vector or mitk::Point.
   * \return A new array holding the elements of vectorOrPoint.
   * \attention The array must be of dimension NVectorDimension!
   * \attention This method implicitly converts between data types.
   */
  template <typename ArrayType, typename TCoordRep, unsigned int NVectorDimension>
  ArrayType ToArray(const itk::FixedArray<TCoordRep, NVectorDimension> &vectorOrPoint)
  {
    ArrayType result;

    mitk::ToArray(result, vectorOrPoint);

    return result;
  }

  /** \brief Fill a 3D array/vector with the given x, y, z values. */
  template <class Tout>
  inline void FillVector3D(Tout &out, mitk::ScalarType x, mitk::ScalarType y, mitk::ScalarType z)
  {
    out[0] = x;
    out[1] = y;
    out[2] = z;
  }

  /** \brief Fill a 4D array/vector with the given x, y, z, t values. */
  template <class Tout>
  inline void FillVector4D(Tout &out, mitk::ScalarType x, mitk::ScalarType y, mitk::ScalarType z, mitk::ScalarType t)
  {
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = t;
  }

  /**
   * \brief Compare two array-like objects element-wise for equality within a tolerance.
   *
   * ArrayTypes are all types that implement operator[]. Pay attention not to set size
   * higher than the actual size of the ArrayType as this will lead to unexpected results.
   *
   * \param[in] arrayType1 First array to compare.
   * \param[in] arrayType2 Second array to compare.
   * \param[in] size Number of elements to compare.
   * \param[in] eps Tolerance for floating point comparison.
   * \param[in] verbose Flag indicating detailed console output.
   * \return True if all compared elements are equal within the given tolerance.
   */
  template <typename TArrayType1, typename TArrayType2>
  inline bool EqualArray(
    TArrayType1 &arrayType1, TArrayType2 &arrayType2, int size, ScalarType eps = mitk::eps, bool verbose = false)
  {
    bool isEqual = true;
    for (int var = 0; var < size; ++var)
    {
      isEqual = isEqual && Equal(arrayType1[var], arrayType2[var], eps);
    }

    ConditionalOutputOfDifference(arrayType1, arrayType2, eps, verbose, isEqual);

    return isEqual;
  }
}

#endif

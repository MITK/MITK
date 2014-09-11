/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKARRAY_H_
#define MITKARRAY_H_

#include <itkFixedArray.h>

#include "mitkNumericConstants.h"
#include "mitkEqual.h"


namespace mitk {


  /**
   *  Methods to copy from itk::FixedArray s (like mitk::Vector and mitk::Point) into ArrayTypes and vice versa.
   *  ArrayTypes here are all types who implement operator[].
   *  The two templated methods were made free floating so you may specialize them
   *  for your concrete type.
   */


  /**
   * @brief Copies elements of an array to this Vector
   * @param[in] array    the array whose values will be copied into the Vector. Must be of a type which overrides the [] operator
   * @param[out] toArray  the FixedArrray (e.g., mitk::Vector or mitk::Point) which should hold the elements of array.
   * @attention array must be of dimension NVectorDimension!
   * @attention this method implicitly converts between data types.
   */
  template <typename ArrayType, typename TCoordRep, unsigned int NVectorDimension>
  void FillArray(itk::FixedArray<TCoordRep, NVectorDimension>& toArray, const ArrayType& array)
  {
    itk::FixedArray<TCoordRep, NVectorDimension> vectorOrPoint;
    for (unsigned short int var = 0; var < NVectorDimension; ++var)
    {
      toArray[var] =  array[var];
    }
  }

  /**
   * @brief Copies elements of an array to this Vector
   * @param[in] array    the array whose values will be copied into the Vector. Must be of a type which overrides the [] operator
   * @param return       the FixedArrray (e.g., mitk::Vector or mitk::Point) which should hold the elements of array.
   * @attention array must be of dimension NVectorDimension!
   * @attention this method implicitly converts between data types.
   */
  template <typename ArrayType, typename TCoordRep, unsigned int NVectorDimension>
  itk::FixedArray<TCoordRep, NVectorDimension> FillArray(const ArrayType& array)
  {
    itk::FixedArray<TCoordRep, NVectorDimension> vectorOrPoint;

    mitk::FillArray(vectorOrPoint, array);

    return vectorOrPoint;
  }

  /**
   * @brief Copies the elements of this into an array
   * @param[in] vectorOrPoint   the itk::FixedArray which shall be copied into the array. Can e.g. be of type mitk::Vector or mitk::Point
   * @param[out] array    the array which will hold the elements. Must be of a type which overrides the [] operator.
   * @attention array must be of dimension NVectorDimension!
   * @attention this method implicitly converts between data types.
   */
  template <typename ArrayType, typename TCoordRep, unsigned int NVectorDimension>
  void ToArray(ArrayType& array, const itk::FixedArray<TCoordRep, NVectorDimension>& vectorOrPoint)
  {
    for (unsigned short int var = 0; var < NVectorDimension; ++var)
    {
      array[var] = vectorOrPoint[var];
    }
  }


  /**
   * @brief Copies the elements of this into an array
   * @param[in] vectorOrPoint   the itk::FixedArray which shall be copied into the array. Can e.g. be of type mitk::Vector or mitk::Point
   * @return    the array which will hold the elements. Must be of a type which overrides the [] operator.
   * @attention array must be of dimension NVectorDimension!
   * @attention this method implicitly converts between data types.
   */
  template <typename ArrayType, typename TCoordRep, unsigned int NVectorDimension>
  ArrayType ToArray(const itk::FixedArray<TCoordRep, NVectorDimension>& vectorOrPoint)
  {
    ArrayType result;

    mitk::ToArray(result, vectorOrPoint);

    return result;
  }


  // The FillVector3D and FillVector4D methods are implemented for all common array types here

  template <class Tout>
  inline void FillVector3D(Tout& out, mitk::ScalarType x, mitk::ScalarType y, mitk::ScalarType z)
  {
    out[0] = x;
    out[1] = y;
    out[2] = z;
  }

  template <class Tout>
  inline void FillVector4D(Tout& out, mitk::ScalarType x, mitk::ScalarType y, mitk::ScalarType z, mitk::ScalarType t)
  {
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = t;
  }



  /**
   * Compares two ArrayTypes of size size.
   * ArrayTypes are all Objects/Types who have a [] operator. Pay attention not to set size higher
   * than the actual size of the ArrayType as this will lead to unexpected results.
   */
  template <typename TArrayType1, typename TArrayType2>
    inline bool EqualArray(TArrayType1& arrayType1, TArrayType2& arrayType2, int size, ScalarType eps = mitk::eps, bool verbose = false)
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


#endif /* MITKARRAY_H_ */

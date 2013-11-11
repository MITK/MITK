/*
 * mitkTypeConversions.h
 *
 *  Created on: Nov 11, 2013
 *      Author: wirkert
 */

#ifndef MITKTYPECONVERSIONS_H_
#define MITKTYPECONVERSIONS_H_


#include "mitkTypeBasics.h"
#include "mitkTypedefs.h"

namespace mitk {


  /**
   *  Now methods to copy from and into ArrayTypes.
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
  void FromArray(itk::FixedArray<TCoordRep, NVectorDimension>& toArray, const ArrayType& array)
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
  itk::FixedArray<TCoordRep, NVectorDimension> FromArray(const ArrayType& array)
  {
    itk::FixedArray<TCoordRep, NVectorDimension> vectorOrPoint;

    mitk::FromArray(vectorOrPoint, array);

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

}


#endif /* MITKTYPECONVERSIONS_H_ */

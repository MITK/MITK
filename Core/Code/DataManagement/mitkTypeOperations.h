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


#ifndef MITKTYPEOPERATIONS_H_
#define MITKTYPEOPERATIONS_H_

#include <itkFixedArray.h>

#include "mitkExceptionMacro.h"

namespace mitk
{
  /**
   * This file provides convenience methods to add and subtract itk::FixedArrays and to multiply and divide itk::FixedArrays with scalars.
   * That means these methods apply to all types deriving from itk::FixedArray.
   * This can e.g. useful to add points and divide them by the number of added points to build an average.
   */

  /**
   * @brief implements sum = addend1 + addend2 for mitk::Point
   *
   * @attention Please make sure, that you really want to add mitk::Point s.
   * @attention E.g., summing two points does geometrically not make sense, which is why itk does
   * @attention not provide an operator+ for these types.
   *
   */
  template< typename TCoordRep, unsigned int NPointDimension>
  mitk::Point<TCoordRep, NPointDimension> operator+(mitk::Point<TCoordRep, NPointDimension>& addend1, mitk::Point<TCoordRep, NPointDimension>& addend2)
  {
    mitk::Point<TCoordRep, NPointDimension> sum;

    for (typename itk::FixedArray<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {
      sum[var] = addend1[var] + addend2[var];
    }

    return sum;
  }


  /**
   * @brief implements difference = minuend - subtrahend for mitk::Point.
   *
   * @return the result of the subtraction
   * @param minuend
   * @param subtrahend
   *
   * @attention Please make sure, that you really want to add mitk::Point s.
   * @attention E.g., summing two points does geometrically not make sense, which is why itk does
   * @attention not provide an operator+ for these types.
   */
  template< typename TCoordRep, unsigned int NPointDimension>
  mitk::Point<TCoordRep, NPointDimension> operator-(mitk::Point<TCoordRep, NPointDimension>& minuend, mitk::Point<TCoordRep, NPointDimension>& subtrahend)
  {
    mitk::Point<TCoordRep, NPointDimension> difference;

    for (typename mitk::Point<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {
      difference[var] = minuend[var] - subtrahend[var];
    }

    return difference;
  }

  /**
   * @brief implements mulPoint = point * scalar for mitk::Point.
   *
   * @return scalar multiplication
   * @param point
   * @param scalar
   *
   * @attention Please make sure, that you really want to add mitk::Point s.
   * @attention E.g., multiplying a points by a scalar does geometrically not make sense, which is why itk does
   * @attention not provide an operator* for these types.
   */
  template< typename TCoordRep, unsigned int NPointDimension>
  mitk::Point<TCoordRep, NPointDimension> operator*(mitk::Point<TCoordRep, NPointDimension>& point, TCoordRep scalar)
  {
    mitk::Point<TCoordRep, NPointDimension> multipliedPoint;

    for (typename mitk::Point<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {
      multipliedPoint[var] = point[var] * scalar;
    }

    return multipliedPoint;
  }

  /**
   * @brief implements divPoint = point / scalar for mitk::Point.
   *
   * @return scalar division
   * @param point
   * @param scalar
   *
   * @attention Make sure you do not divide by zero. Division by zero is not handled by this method
   *
   * @attention Please make sure, that you really want to add mitk::Point s.
   * @attention E.g., dividing a point by a scalar does geometrically not make sense, which is why itk does
   * @attention not provide an operator/ for these types.
   */
  template< typename TCoordRep, unsigned int NPointDimension>
  mitk::Point<TCoordRep, NPointDimension> operator/(mitk::Point<TCoordRep, NPointDimension>& point, TCoordRep scalar)
  {
    mitk::Point<TCoordRep, NPointDimension> dividedPoint;

    for (typename mitk::Point<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {
      dividedPoint[var] = point[var] / scalar;
    }

    return dividedPoint;
  }



}

#endif /* MITKTYPEOPERATIONS_H_ */

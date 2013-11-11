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
   * @brief implements sum = addend1 + addend2
   * Takes FixedArray so you sum all deriving classes like e.g.
   * sum two points returning a point or
   * sum a vector and a point, returning a vector.
   *
   * @attention Please make sure, that you really want to add the types you specified.
   * @attention E.g., summing two points does geometrically not make sense, which is why itk does
   * @attention not provide an operator+ for these types.
   *
   */
  template< typename TCoordRep, unsigned int NPointDimension>
  void add(itk::FixedArray<TCoordRep, NPointDimension>& sum,
      itk::FixedArray<TCoordRep, NPointDimension>& addend1, itk::FixedArray<TCoordRep, NPointDimension>& addend2)
  {
    for (typename itk::FixedArray<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {
      sum[var] = addend1[var] + addend2[var];
    }
  }


  /**
   * @brief implements difference = minuend - subtrahend.
   *
   * @param difference
   * @param minuend
   * @param subtrahend
   */
  template< typename TCoordRep, unsigned int NPointDimension>
  void sub(itk::FixedArray<TCoordRep, NPointDimension>& difference,
      itk::FixedArray<TCoordRep, NPointDimension>& minuend, itk::FixedArray<TCoordRep, NPointDimension>& subtrahend)
  {
    for (typename itk::FixedArray<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {
      difference[var] = minuend[var] - subtrahend[var];
    }
  }


  /**
   * @brief Multiplies a scalar to a FixedArray: product = multiplicand * scalar
   * E.g. multiplying the point {1.0, 2.0, 3.0} with 2.0 results in {2.0, 4.0, 6.0}
   *
   * @param product     {2.0, 4.0, 6.0} in the above example
   * @param multiplicand         {1.0, 2.0, 3.0} in the above example
   * @param scalar      2.0 in the above example
   */
  template< typename TCoordRep, unsigned int NPointDimension>
  void mul(itk::FixedArray<TCoordRep, NPointDimension>& product,
      itk::FixedArray<TCoordRep, NPointDimension>& multiplicand, TCoordRep scalar)
  {
    for (typename itk::FixedArray<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {
      product[var] = multiplicand[var] * scalar;
    }
  }


  /**
   * @brief implements quotient = dividend / scalar.
   * @throws mitk::Exception in case division by zero is attempted
   *
   * @param quotient
   * @param dividend
   * @param scalar
   * @param eps   defines how near to 0 an exception shall be thrown if trying to divide by such a value.
   */
  template< typename TCoordRep, unsigned int NPointDimension>
  void div(itk::FixedArray<TCoordRep, NPointDimension>& quotient,
      itk::FixedArray<TCoordRep, NPointDimension>& dividend, const TCoordRep scalar,
      TCoordRep eps = mitk::eps)
  {
    if (Equal(scalar, 0.0, eps)) // TODO SW: do we want an exception thrown when dividing against zero?
      mitkThrow() << "Division by zero attempted. Scalar to divide is " << scalar;

    for (typename itk::FixedArray<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {

      quotient[var] = dividend[var] / scalar;
    }
  }

}

#endif /* MITKTYPEOPERATIONS_H_ */

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


  template< typename TCoordRep, unsigned int NPointDimension>
  void sub(itk::FixedArray<TCoordRep, NPointDimension>& difference,
      itk::FixedArray<TCoordRep, NPointDimension>& minuend, itk::FixedArray<TCoordRep, NPointDimension>& subtrahend)
  {
    for (typename itk::FixedArray<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {
      difference[var] = minuend[var] - subtrahend[var];
    }
  }


  template< typename TCoordRep, unsigned int NPointDimension>
  void mul(itk::FixedArray<TCoordRep, NPointDimension>& product,
      itk::FixedArray<TCoordRep, NPointDimension>& multiplicand, itk::FixedArray<TCoordRep, NPointDimension>& multiplier)
  {
    for (typename itk::FixedArray<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {
      product[var] = multiplicand[var] * multiplier[var];
    }
  }


  /**
   *
   * @throws mitk::Exception in case division by zero is attempted
   *
   * @param quotient
   * @param dividend
   * @param divisor
   * @param eps   defines how near to 0 an exception shall be thrown if trying to divide by such a value.
   */
  template< typename TCoordRep, unsigned int NPointDimension>
  void div(itk::FixedArray<TCoordRep, NPointDimension>& quotient,
      itk::FixedArray<TCoordRep, NPointDimension>& dividend, itk::FixedArray<TCoordRep, NPointDimension>& divisor,
      TCoordRep eps = mitk::eps)
  {
    for (typename itk::FixedArray<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {
      if (Equal(divisor[var], 0.0, eps))
        mitkThrow() << "Division by zero attempted. 0 found in " << divisor;
      quotient[var] = dividend[var] / divisor[var]; // TODO SW: do we want an exception thrown when dividing against zero?
    }
  }

}

#endif /* MITKTYPEOPERATIONS_H_ */

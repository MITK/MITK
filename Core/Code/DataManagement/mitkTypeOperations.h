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
   * result the result of the subtraction
   * @param minuend
   * @param subtrahend
   */
  template< typename TCoordRep, unsigned int NPointDimension>
  itk::FixedArray<TCoordRep, NPointDimension> sub(itk::FixedArray<TCoordRep, NPointDimension>& minuend, itk::FixedArray<TCoordRep, NPointDimension>& subtrahend)
  {
    itk::FixedArray<TCoordRep, NPointDimension> difference;

    for (typename itk::FixedArray<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
    {
      difference[var] = minuend[var] - subtrahend[var];
    }

    return difference;
  }

}

#endif /* MITKTYPEOPERATIONS_H_ */

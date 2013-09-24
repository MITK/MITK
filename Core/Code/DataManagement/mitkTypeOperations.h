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

/**
 * @brief implements sum = s1 + s2
 * Takes FixedArray so you sum all deriving classes like e.g.
 * sum two points returning a point or
 * sum a vector and a point, returning a vector.
 *
 * @attention Please make sure, that you really want to add the types you specified.
 * @attention E.g., summing two points does geometrically not make sense, which is why itk does
 * @attention not provide an operator+ for these types.
 *
 * @param sum
 * @param s1
 * @param s2
 */
template< typename TCoordRep, unsigned int NPointDimension>
void add(itk::FixedArray<TCoordRep, NPointDimension>& sum,
    itk::FixedArray<TCoordRep, NPointDimension>& s1, itk::FixedArray<TCoordRep, NPointDimension>& s2)
{
  for (typename itk::FixedArray<TCoordRep, NPointDimension>::SizeType var = 0; var < NPointDimension; ++var)
  {
    sum[var] = s1[var] + s2[var];
  }
}


#endif /* MITKTYPEOPERATIONS_H_ */

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


#ifndef MITKTESTING_H_HEADER_INCLUDED
#define MITKTESTING_H_HEADER_INCLUDED

#include <mitkVector.h>
#include <mitkLogMacros.h>
#include <iomanip>

namespace mitk {

/**
 * @brief Equal
 * @param scalar1
 * @param scalar2
 * @param verbose
 * @param eps
 * @return
 */
inline bool Equal(double scalar1, double scalar2, ScalarType eps, bool verbose)
{
  bool isEqual = mitk::Equal(scalar1, scalar2, eps);

  if(( !isEqual ) && verbose)
  {
    MITK_INFO << "Scalars not equal. Righthandside " << std::setprecision(12) << scalar1 << " - Lefthandside " << scalar2 << " - epsilon " << eps;
  }
  return isEqual;
}

template <typename TCoordRep, unsigned int NPointDimension>
inline bool Equal(const itk::Vector<TCoordRep, NPointDimension>& vector1, const itk::Vector<TCoordRep, NPointDimension>& vector2, TCoordRep eps, bool verbose)
{
  bool isEqual = mitk::Equal( vector1, vector2, eps );

  if(( !isEqual ) && verbose)
  {
    MITK_INFO << "Vector not equal. Righthandside " << std::setprecision(12) << vector1 << " - Lefthandside " << vector2 << " - epsilon " << eps;
  }
  return isEqual;
}

template <typename TCoordRep, unsigned int NPointDimension>
inline bool Equal(const itk::Point<TCoordRep, NPointDimension>& vector1, const itk::Point<TCoordRep, NPointDimension>& vector2, TCoordRep eps, bool verbose)
{
  bool isEqual = mitk::Equal( vector1, vector2, eps );

  if(( !isEqual ) && verbose)
  {
    MITK_INFO << "Vector not equal. Righthandside " << std::setprecision(12) << vector1 << " - Lefthandside " << vector2 << " - epsilon " << eps;
  }
  return isEqual;
}

inline bool Equal(const mitk::VnlVector& vector1, const mitk::VnlVector& vector2, bool verbose, ScalarType eps)
{
  bool isEqual = mitk::Equal( vector1, vector2, eps );

  if(( !isEqual ) && verbose)
  {
    MITK_INFO << "Vector not equal. Righthandside " << std::setprecision(12) << vector1 << " - Lefthandside " << vector2 << " - epsilon " << eps;
  }
  return isEqual;
}

template <typename TCoordRep, unsigned int NPointDimension>
inline bool Equal(const vnl_vector_fixed<TCoordRep, NPointDimension> & vector1, const vnl_vector_fixed<TCoordRep, NPointDimension>& vector2, TCoordRep eps, bool verbose)
{
  bool isEqual = mitk::Equal( vector1, vector2, eps );

  if(( !isEqual ) && verbose)
  {
    MITK_INFO << "Vector not equal. Righthandside " << std::setprecision(12) << vector1 << " - Lefthandside " << vector2 << " - epsilon " << eps;
  }
  return isEqual;
}
}

#endif //MITKTESTING_H_HEADER_INCLUDED

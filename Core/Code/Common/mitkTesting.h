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
 * @brief Equal Verbose version of the mitk::Equal method in mitkVector.h
 * @ingroup MITKTestingAPI
 * @param scalar1 Scalar value to compare.
 * @param scalar2 Scalar value to compare.
 * @param eps Tolerance for floating point comparison.
 * @param verbose Flag indicating detailed console output.
 * @return True if scalars are equal.
 */
inline bool Equal(double scalar1, double scalar2, ScalarType eps, bool verbose)
{
  bool isEqual = mitk::Equal(scalar1, scalar2, eps);

  if(( !isEqual ) && verbose)
  {
    MITK_INFO << "Scalars not equal. Lefthandside " << std::setprecision(12) << scalar1 << " - Righthandside " << scalar2 << " - epsilon " << eps;
  }
  return isEqual;
}

/**
 * @brief Equal Verbose version of the mitk::Equal method in mitkVector.h
 * @ingroup MITKTestingAPI
 * @param vector1 Vector to compare.
 * @param vector2 Vector to compare.
 * @param eps Tolerance for floating point comparison.
 * @param verbose Flag indicating detailed console output.
 * @return True if vectors are equal.
 */
template <typename TCoordRep, unsigned int NPointDimension>
inline bool Equal(const itk::Vector<TCoordRep, NPointDimension>& vector1, const itk::Vector<TCoordRep, NPointDimension>& vector2, TCoordRep eps, bool verbose)
{
  bool isEqual = mitk::Equal( vector1, vector2, eps );

  if(( !isEqual ) && verbose)
  {
    MITK_INFO << "Vectors not equal. Lefthandside " << std::setprecision(12) << vector1 << " - Righthandside " << vector2 << " - epsilon " << eps;
  }
  return isEqual;
}

/**
 * @brief Equal Verbose version of the mitk::Equal method in mitkVector.h
 * @ingroup MITKTestingAPI
 * @param point1 Point to compare.
 * @param point2 Point to compare.
 * @param eps Tolerance for floating point comparison.
 * @param verbose Flag indicating detailed console output.
 * @return True if points are equal.
 */
template <typename TCoordRep, unsigned int NPointDimension>
inline bool Equal(const itk::Point<TCoordRep, NPointDimension>& point1, const itk::Point<TCoordRep, NPointDimension>& point2, TCoordRep eps, bool verbose)
{
  bool isEqual = mitk::Equal( point1, point2, eps );

  if(( !isEqual ) && verbose)
  {
    MITK_INFO << "Points not equal. Lefthandside " << std::setprecision(12) << point1 << " - Righthandside " << point2 << " - epsilon " << eps;
  }
  return isEqual;
}

/**
 * @brief Equal Verbose version of the mitk::Equal method in mitkVector.h
 * @ingroup MITKTestingAPI
 * @param vector1 Vector to compare.
 * @param vector2 Vector to compare.
 * @param eps Tolerance for floating point comparison.
 * @param verbose Flag indicating detailed console output.
 * @return True if vectors are equal.
 */
inline bool Equal(const mitk::VnlVector& vector1, const mitk::VnlVector& vector2, bool verbose, ScalarType eps)
{
  bool isEqual = mitk::Equal( vector1, vector2, eps );

  if(( !isEqual ) && verbose)
  {
    MITK_INFO << "Vectors not equal. Lefthandside " << std::setprecision(12) << vector1 << " - Righthandside " << vector2 << " - epsilon " << eps;
  }
  return isEqual;
}

/**
 * @brief Equal Verbose version of the mitk::Equal method in mitkVector.h
 * @ingroup MITKTestingAPI
 * @param vector1 Vector to compare.
 * @param vector2 Vector to compare.
 * @param eps Tolerance for floating point comparison.
 * @param verbose Flag indicating detailed console output.
 * @return True if vectors are equal.
 */
template <typename TCoordRep, unsigned int NPointDimension>
inline bool Equal(const vnl_vector_fixed<TCoordRep, NPointDimension> & vector1, const vnl_vector_fixed<TCoordRep, NPointDimension>& vector2, TCoordRep eps, bool verbose)
{
  bool isEqual = mitk::Equal( vector1, vector2, eps );

  if(( !isEqual ) && verbose)
  {
    MITK_INFO << "Vectors not equal. Lefthandside " << std::setprecision(12) << vector1 << " - Righthandside " << vector2 << " - epsilon " << eps;
  }
  return isEqual;
}
}

#endif //MITKTESTING_H_HEADER_INCLUDED

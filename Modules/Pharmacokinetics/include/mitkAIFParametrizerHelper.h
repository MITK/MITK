/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAIFParametrizerHelper_h
#define mitkAIFParametrizerHelper_h

#include <mitkModelBase.h>

#include <MitkPharmacokineticsExports.h>

namespace mitk
{
  /** \brief Converts an itk::Array<double> into a StaticParameterValuesType (std::vector<double>).
   *
   * Iterates through the array elements and pushes each value into the resulting vector.
   *
   * \param[in] array The source itk::Array to convert.
   * \return A std::vector containing the same values as the input array.
   * \sa convertParameterToArray
   */
  MITKPHARMACOKINETICS_EXPORT ModelBase::StaticParameterValuesType  convertArrayToParameter(itk::Array<double> array);

  /** \brief Converts a StaticParameterValuesType (std::vector<double>) into an itk::Array<double>.
   *
   * Creates an itk::Array of the same size as the input vector and copies each element.
   *
   * \param[in] parameter The source std::vector to convert.
   * \return An itk::Array containing the same values as the input vector.
   * \sa convertArrayToParameter
   */
  MITKPHARMACOKINETICS_EXPORT itk::Array<double> convertParameterToArray(ModelBase::StaticParameterValuesType parameter);
}

#endif

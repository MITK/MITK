/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAIFParametrizerHelper_h
#define mitkAIFParametrizerHelper_h

#include "mitkModelBase.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk
{
  /* Helper function for conversion of an itk::Array into ModelBase::StaticParameterValuesType(std::vector).
    Iterates through array and pushes elements into vector*/
  MITKPHARMACOKINETICS_EXPORT ModelBase::StaticParameterValuesType  convertArrayToParameter(itk::Array<double> array);

  /* Helper function for conversion of a ModelBase::StaticParameterValuesType(std::vector) into an itk::Array
   * Iterates through vector and sets value as array element.*/
  MITKPHARMACOKINETICS_EXPORT itk::Array<double> convertParameterToArray(ModelBase::StaticParameterValuesType);
}

#endif

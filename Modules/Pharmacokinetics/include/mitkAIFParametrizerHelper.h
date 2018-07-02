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

#ifndef __MITK_AIF_PARAMETERIZER_HELPER_H_
#define __MITK_AIF_PARAMETERIZER_HELPER_H_

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

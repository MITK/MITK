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

#ifndef MITKNUMERICCONSTANTS_H_
#define MITKNUMERICCONSTANTS_H_

#include <MitkCoreExports.h>

namespace mitk
{
  typedef double ScalarType;

  MITKCORE_EXPORT extern const ScalarType eps;
  MITKCORE_EXPORT extern const ScalarType sqrteps;
  MITKCORE_EXPORT extern const double large;
}

#endif // MITKNUMERICCONSTANTS_H_

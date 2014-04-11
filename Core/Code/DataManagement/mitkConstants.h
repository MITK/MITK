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


#ifndef MITKDATATYPEBASICS_H
#define MITKDATATYPEBASICS_H

#include <MitkCoreExports.h>

namespace mitk {

typedef double ScalarType;

MITK_CORE_EXPORT extern const ScalarType eps;
MITK_CORE_EXPORT extern const ScalarType sqrteps;
MITK_CORE_EXPORT extern const double large;

}

#endif // MITKDATATYPEBASICS_H

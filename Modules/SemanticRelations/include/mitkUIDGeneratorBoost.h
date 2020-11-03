/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKUIDGENERATORBOOST_H
#define MITKUIDGENERATORBOOST_H

#include <MitkSemanticRelationsExports.h>

#include <iostream>

namespace mitk
{
  namespace UIDGeneratorBoost
  {
    MITKSEMANTICRELATIONS_EXPORT std::string GenerateUID();
  } // namespace UIDGeneratorBoost
} // namespace mitk

#endif // MITKUIDGENERATORBOOST_H

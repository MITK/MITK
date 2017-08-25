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

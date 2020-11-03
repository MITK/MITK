/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSplitParameterToVector_h
#define mitkSplitParameterToVector_h

#include "MitkCLUtilitiesExports.h"
#include <string>
#include <vector>

namespace mitk
{
  namespace cl
  {
    std::vector<double> MITKCLUTILITIES_EXPORT splitDouble(std::string str, char delimiter);
    std::vector<int> MITKCLUTILITIES_EXPORT splitInt(std::string str, char delimiter);
    std::vector<std::string> MITKCLUTILITIES_EXPORT splitString(std::string str, char delimiter);
  }
}


#endif //mitkSplitParameterToVector_h

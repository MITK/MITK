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

#ifndef mitkSplitParameterToVector_h
#define mitkSplitParameterToVector_h

#include <string>
#include <vector>

namespace mitk
{
  namespace cl
  {
    std::vector<double> splitDouble(std::string str, char delimiter);
    std::vector<int> splitInt(std::string str, char delimiter);
    std::vector<std::string> splitString(std::string str, char delimiter);
  }
}


#endif //mitkSplitParameterToVector_h
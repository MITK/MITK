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

#ifndef MITKRESTUTIL_H
#define MITKRESTUTIL_H

#include "MitkCppRestSdkExports.h"

#include <iostream>
#include "cpprest/asyncrt_utils.h"

namespace mitk
{
  class MITKCPPRESTSDK_EXPORT RESTUtil
  {

  public:

    static std::string convertToUtf8(utility::string_t stringT) { return utility::conversions::to_utf8string(stringT); }
	   static utility::string_t convertToTString(std::string string) { return utility::conversions::to_string_t(string); }

  };
};

#endif // MITKRESTUTIL_H

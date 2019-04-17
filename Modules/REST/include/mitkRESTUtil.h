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

#ifndef mitkRESTUtil_h
#define mitkRESTUtil_h

#include <MitkRESTExports.h>
#include <cpprest/asyncrt_utils.h>

namespace mitk
{
  class MITKREST_EXPORT RESTUtil
  {
  public:
    /**
     * @brief Converts the given std::wstring into a std::string representation
     */
    static std::string convertToUtf8(const utility::string_t &string)
    {
      return utility::conversions::to_utf8string(string);
    }

	  /**
	   * @brief Converts the given std::string into a std::wstring representation
	   */
    static utility::string_t convertToTString(const std::string &string)
    {
      return utility::conversions::to_string_t(string);
    }
  };
}

#endif

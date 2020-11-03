/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRESTUtil_h
#define mitkRESTUtil_h

#include <MitkRESTExports.h>
#include <cpprest/asyncrt_utils.h>
#include <map>


namespace mitk
{
  class MITKREST_EXPORT RESTUtil
  {
  public:

    typedef std::map<utility::string_t, utility::string_t> ParamMap;

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

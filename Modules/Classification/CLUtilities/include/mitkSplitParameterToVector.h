/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSplitParameterToVector_h
#define mitkSplitParameterToVector_h

#include <MitkCLUtilitiesExports.h>
#include <string>
#include <vector>

namespace mitk
{
  namespace cl
  {
    /**
     * \brief Split a delimited string into a vector of doubles.
     * \param str The input string (e.g. "1.0;2.5;3.0").
     * \param delimiter The delimiter character.
     * \return Vector of parsed double values.
     */
    std::vector<double> MITKCLUTILITIES_EXPORT splitDouble(std::string str, char delimiter);

    /**
     * \brief Split a delimited string into a vector of integers.
     * \param str The input string (e.g. "1;2;3").
     * \param delimiter The delimiter character.
     * \return Vector of parsed integer values.
     */
    std::vector<int> MITKCLUTILITIES_EXPORT splitInt(std::string str, char delimiter);

    /**
     * \brief Split a delimited string into a vector of substrings.
     * \param str The input string.
     * \param delimiter The delimiter character.
     * \return Vector of substring tokens.
     */
    std::vector<std::string> MITKCLUTILITIES_EXPORT splitString(std::string str, char delimiter);
  }
}


#endif

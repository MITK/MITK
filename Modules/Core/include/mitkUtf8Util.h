/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUtf8Util_h
#define mitkUtf8Util_h

#include <MitkCoreExports.h>
#include <string>

namespace mitk
{
  /** \brief Utility functions for UTF-8 string conversion. */
  namespace Utf8Util
  {
    /**
     * \brief Convert a string encoded with the current code page to a UTF-8 encoded string (Windows).
     *
     * The conversion happens on Windows only. On all other platforms, the input string
     * is returned unmodified as it is assumed to be UTF-8 encoded already.
     *
     * If the conversion fails, a warning is printed and the input string is returned
     * instead. This matches the behavior before this method was introduced.
     *
     * \param[in] local8BitStr The string in the current system code page encoding.
     * \return The UTF-8 encoded string.
     */
    MITKCORE_EXPORT std::string Local8BitToUtf8(const std::string& local8BitStr);

    /**
     * \brief Convert a UTF-8 encoded string to a string encoded with the current code page (Windows).
     *
     * The conversion happens on Windows only. On all other platforms, the input string
     * is returned unmodified as strings are assumed to be always UTF-8 encoded by default.
     *
     * If the conversion fails, a warning is printed and the input string is returned
     * instead. This matches the behavior before this method was introduced.
     *
     * \param[in] utf8Str The UTF-8 encoded string.
     * \return The string in the current system code page encoding.
     */
    MITKCORE_EXPORT std::string Utf8ToLocal8Bit(const std::string& utf8Str);
  }
}

#endif

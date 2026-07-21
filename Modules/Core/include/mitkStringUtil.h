/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStringUtil_h
#define mitkStringUtil_h

#include <MitkCoreExports.h>

#include <string>
#include <vector>

namespace mitk
{
  /**
   * \brief Case-insensitive check whether \a str ends with \a suffix.
   *
   * Only the trailing characters of \a str are compared against \a suffix, without
   * regard to case, so \a suffix may be given in any case. Useful to route files by
   * filename suffix (for example ".mitkscene.json" or ".json"), where
   * std::filesystem::path::extension() is insufficient because it returns only the
   * last extension.
   *
   * An empty suffix matches any string. A suffix longer than \a str never matches.
   */
  MITKCORE_EXPORT bool EndsWithCaseInsensitive(const std::string& str, const std::string& suffix);

  /**
   * \brief Case-insensitive equality check of \a lhs and \a rhs.
   *
   * Returns true only when both strings have the same length and their characters
   * match without regard to case.
   */
  MITKCORE_EXPORT bool EqualsCaseInsensitive(const std::string& lhs, const std::string& rhs);

  /**
   * \brief Split \a str into substrings on each occurrence of \a delimiter.
   *
   * Empty fields are preserved, so the result contains one more element than the
   * number of delimiters in \a str (an empty \a str yields a single empty string).
   */
  MITKCORE_EXPORT std::vector<std::string> Split(const std::string& str, char delimiter);

  /**
   * \brief Remove leading and trailing whitespace from \a str in place.
   */
  MITKCORE_EXPORT void Trim(std::string& str);

  /**
   * \brief Replace every non-overlapping occurrence of \a from with \a to in \a str, in place.
   *
   * Does nothing if \a from is empty.
   */
  MITKCORE_EXPORT void ReplaceAll(std::string& str, const std::string& from, const std::string& to);
}

#endif

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSceneFileUtil_h
#define mitkSceneFileUtil_h

#include <MitkSceneSerializationExports.h>

#include <string>

namespace mitk
{
  /**
   * \brief Case-insensitive check whether \a str ends with \a suffix.
   *
   * Both operands are lowercased before comparison, so \a suffix may be given in
   * any case. Used to route scene files by filename suffix (for example
   * ".mitkscene.json" or ".json"), where std::filesystem::path::extension() is
   * insufficient because it returns only the last extension.
   *
   * An empty suffix matches any string. A suffix longer than \a str never matches.
   */
  MITKSCENESERIALIZATION_EXPORT bool EndsWithCaseInsensitive(const std::string& str, const std::string& suffix);
}

#endif

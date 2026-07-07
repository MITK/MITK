/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPythonUtil_h
#define mitkPythonUtil_h

#include <MitkPythonExports.h>

#include <string>

namespace mitk
{
  /**
   * \brief Formats a string as a Python single-quoted string literal.
   *
   * Backslashes and single quotes are escaped so the value round-trips safely
   * when spliced into generated Python source (for example a Windows path like
   * C:\\foo\\bar, or a user-edited preference). Use this for every untrusted
   * value that ends up in code run through the interpreter, so a stray quote or
   * backslash cannot break the generated Python or inject code.
   */
  MITKPYTHON_EXPORT std::string PyQuote(const std::string& value);
}

#endif

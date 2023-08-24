/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLogMessage.h>

mitk::LogMessage::LogMessage(const LogLevel level, const std::string& filePath, const int lineNumber,
                             const std::string& functionName)
  : Level(level),
    FilePath(filePath),
    LineNumber(lineNumber),
    FunctionName(functionName)
{
}

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPostInstallStepProgress.h>

#include <regex>
#include <stdexcept>

std::optional<mitk::PostInstallStepProgress> mitk::ParsePostInstallStepProgress(const std::string& line)
{
  static const std::regex regex(R"(^MITK_PROGRESS (\d+) (\d+)\s*$)");
  std::smatch match;

  if (!std::regex_match(line, match, regex))
    return std::nullopt;

  try
  {
    return PostInstallStepProgress{ std::stoull(match[1].str()), std::stoull(match[2].str()) };
  }
  catch (const std::out_of_range&)
  {
    return std::nullopt;
  }
}

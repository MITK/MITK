/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPostInstallStepProgress_h
#define mitkPostInstallStepProgress_h

#include <MitkPythonInstallerExports.h>

#include <cstdint>
#include <optional>
#include <string>

namespace mitk
{
  /**
   * \brief A progress report of a post-install step.
   *
   * A step reports its progress by printing lines of the form
   * <tt>MITK_PROGRESS <done> <total></tt> to its standard output, with
   * non-negative integers in a unit of its choice, for example the bytes of a
   * download. A total of 0 tells that the step is busy without knowing how far
   * it is. A step that turns out to have nothing to do prints no such line.
   *
   * Whoever runs a step shows these lines as its progress instead of passing
   * them on as output.
   *
   * \sa PostInstallStep, ParsePostInstallStepProgress()
   */
  struct PostInstallStepProgress
  {
    std::uint64_t Done = 0;
    std::uint64_t Total = 0;
  };

  /**
   * \brief Reads a progress report from a line of the output of a post-install step.
   *
   * \param[in] line A line of the standard output, with or without its line break.
   *
   * \return The report, or \c std::nullopt if the line is not one.
   */
  MITKPYTHONINSTALLER_EXPORT std::optional<PostInstallStepProgress> ParsePostInstallStepProgress(const std::string& line);
}

#endif

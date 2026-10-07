/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCrashDumpSessionOptions.h>

#include "mitkCrashDumpSessionOptionsRecord.h"

namespace
{
  mitk::CrashDumpSessionOptions s_SessionOptions;
}

mitk::CrashDumpSessionOptions mitk::ResolveCrashDumpSessionOptions(
  const CrashDumpOptionOverrides& overrides, const CrashDumpSettings& settings)
{
  CrashDumpSessionOptions options;

  if (overrides.NoCrashDumpsFlag)
  {
    options.Arm = false;
    options.ArmSource = CrashDumpOptionSource::CommandLine;
  }
  else if (overrides.NoCrashDumpsEnvironment)
  {
    options.Arm = false;
    options.ArmSource = CrashDumpOptionSource::EnvironmentVariable;
  }
  else
  {
    options.Arm = settings.Enabled;
    options.ArmSource = CrashDumpOptionSource::Settings;
  }

  // Whoever passes the option or sets the variable is the one debugging this
  // session, so either wins over the persistent setting.
  if (overrides.WatchdogFlagSeconds.has_value())
  {
    options.WatchdogTimeoutSeconds = *overrides.WatchdogFlagSeconds;
    options.WatchdogSource = CrashDumpOptionSource::CommandLine;
  }
  else if (overrides.WatchdogEnvironmentSeconds.has_value())
  {
    options.WatchdogTimeoutSeconds = *overrides.WatchdogEnvironmentSeconds;
    options.WatchdogSource = CrashDumpOptionSource::EnvironmentVariable;
  }
  else
  {
    options.WatchdogTimeoutSeconds = settings.WatchdogTimeoutSeconds;
    options.WatchdogSource = CrashDumpOptionSource::Settings;
  }

  return options;
}

mitk::CrashDumpSessionOptions mitk::GetCrashDumpSessionOptions()
{
  return s_SessionOptions;
}

void mitk::RecordCrashDumpSessionOptions(const CrashDumpSessionOptions& options)
{
  s_SessionOptions = options;
}

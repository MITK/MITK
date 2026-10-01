/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCrashDumpSessionOptions_h
#define mitkCrashDumpSessionOptions_h

#include <MitkAppUtilExports.h>

#include <mitkCrashDumpFacility.h>

#include <optional>

namespace mitk
{
  /** \brief Where a crash-dump option in effect for this session came from. */
  enum class CrashDumpOptionSource
  {
    Settings,            /**< The settings file, or its defaults. */
    EnvironmentVariable, /**< MITK_NO_CRASH_DUMPS / MITK_UI_WATCHDOG. */
    CommandLine          /**< --MITK.no-crash-dumps / --MITK.ui-watchdog. */
  };

  /** \brief The per-session overrides found at startup. */
  struct CrashDumpOptionOverrides
  {
    bool NoCrashDumpsFlag = false;
    bool NoCrashDumpsEnvironment = false;
    /** Value of --MITK.ui-watchdog, if given. */
    std::optional<int> WatchdogFlagSeconds;
    /** Value of MITK_UI_WATCHDOG, if set. */
    std::optional<int> WatchdogEnvironmentSeconds;
  };

  /** \brief Crash-dump options a session runs with, and their sources. */
  struct CrashDumpSessionOptions
  {
    bool Arm = true;
    CrashDumpOptionSource ArmSource = CrashDumpOptionSource::Settings;
    /** 0 means no watchdog. */
    int WatchdogTimeoutSeconds = 0;
    CrashDumpOptionSource WatchdogSource = CrashDumpOptionSource::Settings;
  };

  /**
   * \brief Apply the startup precedence: command line > environment variable >
   *        settings file.
   *
   * Overrides only disable crash dumps; an override of the watchdog wins with
   * any value, including 0 (off). Override values are taken as given, unlike
   * the settings file's, so that a developer can choose a timeout below the
   * settings range for a debugging session.
   */
  MITKAPPUTIL_EXPORT CrashDumpSessionOptions ResolveCrashDumpSessionOptions(
    const CrashDumpOptionOverrides& overrides, const CrashDumpSettings& settings);

  /** \brief The options this session runs with, as resolved by
   *  mitk::BaseApplication at startup. Defaults (settings, armed, no
   *  watchdog) if crash handling was never initialized. */
  MITKAPPUTIL_EXPORT CrashDumpSessionOptions GetCrashDumpSessionOptions();
}

#endif

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCrashDumpFacility.h"

#include "mitkCrashDumpDatabase.h"

#include <mitkLog.h>

#include <sentry.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace
{
  constexpr std::size_t kMaxRetainedDumps = 20;

  // Provided by the module CMake (single source of truth, including the
  // platform executable suffix). The fallback only matters in a build that
  // does not place a handler at all, in which case arming fails regardless.
#ifndef MITK_CRASH_HANDLER_FILENAME
#define MITK_CRASH_HANDLER_FILENAME "MitkCrashHandler"
#endif

  // Renamed from the upstream "crashpad_handler" so the out-of-process
  // handler is recognizable as MITK's in a task manager. Because the name
  // differs from what the sentry backend auto-resolves, handler_path must
  // be set explicitly (hence the current-executable lookup below).
  const std::filesystem::path kHandlerFileName = MITK_CRASH_HANDLER_FILENAME;

  struct FacilityState
  {
    std::filesystem::path DatabaseDirectory;
    bool Active = false;
    bool CrashedLastRun = false;
  };

  FacilityState s_State;

  std::filesystem::path CurrentExecutablePath()
  {
#if defined(_WIN32)
    std::wstring buffer(MAX_PATH, L'\0');
    for (;;)
    {
      const auto length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
      if (length == 0)
        return {};
      if (length < buffer.size())
      {
        buffer.resize(length);
        return std::filesystem::path(buffer);
      }
      buffer.resize(buffer.size() * 2); // path did not fit; grow and retry
    }
#elif defined(__APPLE__)
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string buffer(size, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) != 0)
      return {};
    std::error_code error;
    auto resolved = std::filesystem::canonical(std::filesystem::path(buffer), error);
    return error ? std::filesystem::path(buffer) : resolved;
#else
    std::error_code error;
    auto path = std::filesystem::read_symlink("/proc/self/exe", error);
    return error ? std::filesystem::path{} : path;
#endif
  }
}

bool mitk::CrashDumpFacility::Initialize(const Config& config) noexcept
{
  try
  {
    if (s_State.Active)
    {
      MITK_WARN << "Crash-dump facility is already initialized.";
      return true;
    }

    if (config.DatabaseDirectory.empty())
    {
      MITK_WARN << "Crash-dump facility not armed: no database directory configured.";
      return false;
    }

    s_State.DatabaseDirectory = config.DatabaseDirectory;

    std::error_code error;
    std::filesystem::create_directories(s_State.DatabaseDirectory, error);
    if (error)
    {
      MITK_WARN << "Crash-dump facility not armed: cannot create database directory '"
                << s_State.DatabaseDirectory.string() << "': " << error.message();
      return false;
    }

    auto* options = sentry_options_new();

    // Deliberately no sentry_options_set_dsn: without a DSN the crashpad
    // handler is started with an empty upload URL, so it has no endpoint
    // to upload to. Together with SENTRY_TRANSPORT=none at build time this
    // is what makes the facility local-only.
#ifdef _WIN32
    sentry_options_set_database_pathw(options, s_State.DatabaseDirectory.c_str());
#else
    sentry_options_set_database_path(options, s_State.DatabaseDirectory.c_str());
#endif

    const auto release = config.ApplicationName + " " + config.ApplicationVersion;
    sentry_options_set_release(options, release.c_str());

    // No transport is compiled in, so session envelopes could never be
    // sent and would only accumulate in the database.
    sentry_options_set_auto_session_tracking(options, 0);

    const auto executableDirectory = CurrentExecutablePath().parent_path();
    if (executableDirectory.empty())
    {
      MITK_WARN << "Crash-dump facility not armed: cannot locate the current executable.";
      sentry_options_free(options);
      return false;
    }

    const auto handlerPath = executableDirectory / kHandlerFileName;
#if defined(_WIN32)
    sentry_options_set_handler_pathw(options, handlerPath.c_str());
#else
    sentry_options_set_handler_path(options, handlerPath.c_str());
#endif

    if (sentry_init(options) != 0)
    {
      MITK_WARN << "Crash-dump facility failed to arm; is '" << kHandlerFileName.string()
                << "' located next to the application executable?";
      return false;
    }

    s_State.Active = true;
    s_State.CrashedLastRun = sentry_get_crashed_last_run() == 1;

    PruneCrashDumps(s_State.DatabaseDirectory, kMaxRetainedDumps);

    return true;
  }
  catch (...)
  {
    return false;
  }
}

void mitk::CrashDumpFacility::Shutdown() noexcept
{
  if (s_State.Active)
  {
    sentry_close();
    s_State.Active = false;
  }
}

bool mitk::CrashDumpFacility::IsActive() noexcept
{
  return s_State.Active;
}

bool mitk::CrashDumpFacility::CrashedLastRun() noexcept
{
  return s_State.CrashedLastRun;
}

void mitk::CrashDumpFacility::ClearCrashedLastRun()
{
  s_State.CrashedLastRun = false;

  if (s_State.Active)
    sentry_clear_crashed_last_run();

  const auto dumps = ScanCrashDumps(s_State.DatabaseDirectory);
  if (!dumps.empty())
    WriteLastAcknowledgedTime(s_State.DatabaseDirectory, dumps.front().LastWriteTime);
}

std::vector<mitk::CrashDumpInfo> mitk::CrashDumpFacility::ListDumps()
{
  return ScanCrashDumps(s_State.DatabaseDirectory);
}

std::vector<mitk::CrashDumpInfo> mitk::CrashDumpFacility::ListUnacknowledgedDumps()
{
  return ScanUnacknowledgedCrashDumps(s_State.DatabaseDirectory);
}

bool mitk::CrashDumpFacility::DeleteDump(const std::filesystem::path& dumpPath)
{
  std::error_code error;
  return std::filesystem::remove(dumpPath, error) && !error;
}

std::filesystem::path mitk::CrashDumpFacility::GetDatabaseDirectory()
{
  return s_State.DatabaseDirectory;
}

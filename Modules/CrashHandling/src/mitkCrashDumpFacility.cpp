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

#include <client/crashpad_client.h>
#include <util/misc/capture_context.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#include <chrono>
#include <mutex>
#include <set>
#include <thread>

namespace
{
  constexpr std::size_t kMaxRetainedDumps = 10;

  // Facility-owned subdirectories of the database. Snapshots taken via
  // CaptureSnapshot are moved out of Crashpad's report area into one of these
  // so their kind is a filesystem fact that survives a hard kill: on-demand
  // snapshots are never surfaced, and provisional (watchdog) snapshots are
  // purged unless the process is hard-killed mid-freeze.
  const std::filesystem::path kSnapshotsSubdir = "mitk-snapshots";
  const std::filesystem::path kPendingFreezeSubdir = "mitk-pending-freeze";

  std::filesystem::path SubdirForKind(mitk::SnapshotKind kind)
  {
    return mitk::SnapshotKind::OnDemand == kind ? kSnapshotsSubdir : kPendingFreezeSubdir;
  }

  // Serializes CaptureSnapshot / PurgeProvisionalSnapshots, which may run on
  // the watchdog thread and the UI thread concurrently.
  std::mutex s_SnapshotMutex;

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
    // Provisional (watchdog) snapshots this process produced. Tracked in
    // memory so a recovery or clean shutdown purges exactly this session's
    // provisional dumps and never a previous hard-kill survivor the user may
    // still want. A hard kill loses the list, so those dumps persist on disk.
    std::vector<std::filesystem::path> ProvisionalSnapshots;
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

    // Bound only the crash-report area; the snapshot subdirectories keep their
    // own retention (see CaptureSnapshot) and pending-freeze survivors must
    // not be evicted here before the next-start dialog can surface them.
    PruneCrashDumps(s_State.DatabaseDirectory, kMaxRetainedDumps,
      { kSnapshotsSubdir, kPendingFreezeSubdir });

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
    // A clean shutdown means any provisional freeze snapshot was a false
    // positive (the process was not hard-killed mid-freeze), so drop them.
    PurgeProvisionalSnapshots();

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

  // Watermark from the newest surfacable dump, so an on-demand snapshot
  // (excluded from the surfacable set) can never mask a real crash dump.
  const auto dumps = ScanCrashDumps(s_State.DatabaseDirectory, { kSnapshotsSubdir });
  if (!dumps.empty())
    WriteLastAcknowledgedTime(s_State.DatabaseDirectory, dumps.front().LastWriteTime);
}

std::vector<mitk::CrashDumpInfo> mitk::CrashDumpFacility::ListDumps()
{
  return ScanCrashDumps(s_State.DatabaseDirectory, { kSnapshotsSubdir });
}

std::vector<mitk::CrashDumpInfo> mitk::CrashDumpFacility::ListUnacknowledgedDumps()
{
  return ScanUnacknowledgedCrashDumps(s_State.DatabaseDirectory, { kSnapshotsSubdir });
}

std::vector<mitk::CrashDumpInfo> mitk::CrashDumpFacility::ListSnapshots(SnapshotKind kind)
{
  const auto& database = s_State.DatabaseDirectory;

  if (database.empty())
    return {};

  return ScanCrashDumps(database / SubdirForKind(kind));
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

std::optional<std::filesystem::path> mitk::CrashDumpFacility::CaptureSnapshot(SnapshotKind kind)
{
  std::lock_guard<std::mutex> lock(s_SnapshotMutex);

  if (!s_State.Active)
  {
    MITK_WARN << "Cannot capture a diagnostic snapshot: the crash-dump facility is not active.";
    return std::nullopt;
  }

  const auto& database = s_State.DatabaseDirectory;

  // DumpWithoutCrash writes into Crashpad's report area; diff it against the
  // set present before the call to find the freshly written file. The two
  // facility subdirectories are excluded so a previously filed snapshot is
  // never mistaken for the new one.
  const std::vector<std::filesystem::path> reportArea = { kSnapshotsSubdir, kPendingFreezeSubdir };

  std::set<std::filesystem::path> before;
  for (const auto& dump : ScanCrashDumps(database, reportArea))
    before.insert(dump.Path);

  crashpad::NativeCPUContext context;
  crashpad::CaptureContext(&context);
#if defined(_WIN32)
  crashpad::CrashpadClient::DumpWithoutCrash(context);
#else
  crashpad::CrashpadClient::DumpWithoutCrash(&context);
#endif

  // The handler writes out of process and asynchronously, so poll briefly.
  std::filesystem::path newDump;
  for (int attempt = 0; attempt < 50 && newDump.empty(); ++attempt)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (const auto& dump : ScanCrashDumps(database, reportArea))
    {
      if (before.find(dump.Path) == before.end())
      {
        newDump = dump.Path;
        break;
      }
    }
  }

  if (newDump.empty())
  {
    MITK_WARN << "Diagnostic snapshot requested but no minidump was produced.";
    return std::nullopt;
  }

  // Move the dump into its kind's subdirectory so the kind survives a hard
  // kill. Same volume as the source, so rename does not cross devices.
  const auto destinationDir = database / SubdirForKind(kind);
  std::error_code error;
  std::filesystem::create_directories(destinationDir, error);

  const auto destination = destinationDir / newDump.filename();
  std::filesystem::rename(newDump, destination, error);
  if (error)
  {
    MITK_WARN << "Could not file the diagnostic snapshot under '" << destinationDir.string()
              << "': " << error.message();
    std::filesystem::remove(newDump, error); // avoid a stray dump surfacing as a crash
    return std::nullopt;
  }

  if (SnapshotKind::WatchdogProvisional == kind)
    s_State.ProvisionalSnapshots.push_back(destination);

  PruneCrashDumps(destinationDir, kMaxRetainedDumps);

  return destination;
}

void mitk::CrashDumpFacility::PurgeProvisionalSnapshots()
{
  std::lock_guard<std::mutex> lock(s_SnapshotMutex);

  for (const auto& snapshot : s_State.ProvisionalSnapshots)
  {
    std::error_code error;
    std::filesystem::remove(snapshot, error);
  }

  s_State.ProvisionalSnapshots.clear();
}

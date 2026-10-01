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
#else
#include <unistd.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#endif

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <set>
#include <sstream>
#include <thread>

namespace
{
  // On-demand snapshots are never surfaced by the next-start dialog, and
  // provisional (watchdog) snapshots are purged unless the process is
  // hard-killed mid-freeze (see mitkCrashDumpDatabase.h for the areas).
  const std::filesystem::path& kSnapshotsSubdir = mitk::OnDemandSnapshotsSubdir;
  const std::filesystem::path& kPendingFreezeSubdir = mitk::PendingFreezeSubdir;

  // Crashpad's POSIX database stages a dump here and moves it into the report
  // area only once it is complete. Taking one out from under the handler
  // yields a truncated dump and breaks the handler's move. The Windows backend
  // has no staging directory, so there excluding this name matches nothing.
  const std::filesystem::path kCrashpadStagingSubdir = "new";

  std::filesystem::path SubdirForKind(mitk::SnapshotKind kind)
  {
    return mitk::SnapshotKind::OnDemand == kind ? kSnapshotsSubdir : kPendingFreezeSubdir;
  }

  // Serializes CaptureSnapshot / PurgeProvisionalSnapshots, which may run on
  // the watchdog thread and the UI thread concurrently, and guards the
  // provisional-snapshot list the listing calls read.
  std::mutex s_SnapshotMutex;

  // Serializes rewrites of the run-info file.
  std::mutex s_RunInfoMutex;

  // Provided by the build system, which is the single source of truth for the
  // name (including the platform executable suffix) and for every rule that
  // places the file. The fallback only matters in a build that does not place
  // a handler at all, in which case arming fails regardless.
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
    mitk::CrashDumpSettings Settings;
    // The handler copies this file into every report it writes, reading it at
    // capture time, so rewriting it later (SetSessionLogFile) still reaches
    // dumps taken afterwards. It is per process because the database is
    // shared by every running instance.
    std::filesystem::path RunInfoFile;
    mitk::CrashRunInfo RunInfo;
    // Provisional (watchdog) snapshots this process produced. Tracked in
    // memory so a recovery or clean shutdown purges exactly this session's
    // provisional dumps and never a previous hard-kill survivor the user may
    // still want. A hard kill loses the list, so those dumps persist on disk.
    std::vector<std::filesystem::path> ProvisionalSnapshots;
  };

  FacilityState s_State;

  // Crashpad names every report of a session after the same report ID, so the
  // capture time is what keeps repeated snapshots apart - and reading their
  // spacing off the file names is what tells a permanent wedge from a
  // slow-moving loop when one freeze episode produces several dumps.
  [[maybe_unused]] std::string CaptureTimestamp()
  {
    const auto now = std::chrono::system_clock::now();
    const auto whole = std::chrono::time_point_cast<std::chrono::seconds>(now);
    const auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - whole).count();

    const auto time = std::chrono::system_clock::to_time_t(whole);
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif

    char date[16];
    std::strftime(date, sizeof(date), "%Y%m%dT%H%M%S", &utc);

    std::ostringstream timestamp;
    timestamp << date << '.' << std::setw(3) << std::setfill('0') << milliseconds << 'Z';

    return timestamp.str();
  }

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

  /** Outside the database, which every instance shares; a crashed session's
   *  leftover is harmless and a reused process ID merely overwrites it. */
  std::filesystem::path RunInfoFileForThisProcess()
  {
    std::error_code error;
    const auto temporaryDirectory = std::filesystem::temp_directory_path(error);
    if (error)
      return {};

#if defined(_WIN32)
    const auto processId = static_cast<unsigned long>(GetCurrentProcessId());
#else
    const auto processId = static_cast<unsigned long>(getpid());
#endif

    return temporaryDirectory / ("mitk-crash-run-info-" + std::to_string(processId)) /
      mitk::RunInfoAttachmentFileName;
  }

  std::set<std::filesystem::path> ProvisionalSnapshotsOfThisSession()
  {
    std::lock_guard<std::mutex> lock(s_SnapshotMutex);
    return { s_State.ProvisionalSnapshots.begin(), s_State.ProvisionalSnapshots.end() };
  }

  std::vector<mitk::CrashDumpInfo> WithoutProvisionalSnapshotsOfThisSession(std::vector<mitk::CrashDumpInfo> dumps)
  {
    const auto provisional = ProvisionalSnapshotsOfThisSession();

    std::erase_if(dumps, [&provisional](const mitk::CrashDumpInfo& dump) {
      return provisional.find(dump.Path) != provisional.end();
    });

    return dumps;
  }

  /** Crash dumps get their sidecar lazily, from the report's attachment:
   *  the handler writes it after this process is gone, and another instance
   *  sharing the database can crash at any time. */
  std::vector<mitk::CrashDumpInfo> WithRunInfo(std::vector<mitk::CrashDumpInfo> dumps)
  {
    for (auto& dump : dumps)
    {
      if (mitk::DumpKind::Crash == dump.Kind)
        mitk::AdoptRunInfoAttachment(s_State.DatabaseDirectory, dump.Path.stem(), dump.Path);

      mitk::LoadRunInfo(dump);
    }

    return dumps;
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

    // Tests initialize repeatedly in one process; nothing of an earlier
    // session may leak into this one.
    s_State = FacilityState();

    if (config.DatabaseDirectory.empty())
    {
      MITK_WARN << "Crash-dump facility not armed: no database directory configured.";
      return false;
    }

    std::error_code error;
    std::filesystem::create_directories(config.DatabaseDirectory, error);
    if (error)
    {
      MITK_WARN << "Crash-dump facility not armed: cannot create database directory '"
                << config.DatabaseDirectory.string() << "': " << error.message();
      return false;
    }

    // Recorded before any decision not to arm, so that dumps already on disk
    // stay listable and deletable in a session that records no new ones.
    s_State.DatabaseDirectory = config.DatabaseDirectory;
    s_State.Settings = ReadCrashDumpSettings(s_State.DatabaseDirectory);

    const auto maxDumps = static_cast<std::size_t>(s_State.Settings.MaxDumpsPerKind);

    // Bounding the pending-freeze area here cannot hide a hard-killed freeze
    // from the next-start dialog: only the oldest go, and they go only when
    // MaxDumpsPerKind newer ones remain to be shown.
    PruneCrashDumps(s_State.DatabaseDirectory, maxDumps,
      { kSnapshotsSubdir, kPendingFreezeSubdir, kCrashpadStagingSubdir });
    PruneCrashDumps(s_State.DatabaseDirectory / kSnapshotsSubdir, maxDumps);
    PruneCrashDumps(s_State.DatabaseDirectory / kPendingFreezeSubdir, maxDumps);

    if (!config.Arm)
    {
      MITK_INFO << "Crash-dump facility not armed: disabled for this session.";
      return false;
    }

    if (!s_State.Settings.Enabled)
    {
      MITK_INFO << "Crash-dump facility not armed: disabled in '"
                << GetSettingsFilePath(s_State.DatabaseDirectory).string() << "'.";
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

    // Missing run info costs the dump its context, not its capture, so a
    // failure here does not stop arming.
    s_State.RunInfo.Release = release;
    s_State.RunInfo.InstallDirectory = executableDirectory;
    const auto runInfoFile = RunInfoFileForThisProcess();
    std::filesystem::create_directories(runInfoFile.parent_path(), error);

    if (!runInfoFile.empty() && WriteRunInfo(runInfoFile, s_State.RunInfo))
    {
      s_State.RunInfoFile = runInfoFile;
#if defined(_WIN32)
      sentry_options_add_attachmentw(options, runInfoFile.c_str());
#else
      sentry_options_add_attachment(options, runInfoFile.c_str());
#endif
    }
    else
    {
      MITK_WARN << "Crash-dump facility: cannot write the run-info file '" << runInfoFile.string()
                << "'; dumps of this session will carry no version or log information.";
    }

    if (sentry_init(options) != 0)
    {
      MITK_WARN << "Crash-dump facility failed to arm; is the out-of-process handler present at '"
                << handlerPath.string() << "'?";
      std::filesystem::remove_all(runInfoFile.parent_path(), error);
      s_State.RunInfoFile.clear();
      return false;
    }

    s_State.Active = true;
    s_State.CrashedLastRun = sentry_get_crashed_last_run() == 1;

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

    if (!s_State.RunInfoFile.empty())
    {
      std::error_code error;
      std::filesystem::remove_all(s_State.RunInfoFile.parent_path(), error);
      s_State.RunInfoFile.clear();
    }
  }
}

mitk::CrashDumpSettings mitk::CrashDumpFacility::GetSettings() noexcept
{
  return s_State.Settings;
}

mitk::CrashDumpSettings mitk::CrashDumpFacility::ReadSettings()
{
  return ReadCrashDumpSettings(s_State.DatabaseDirectory);
}

bool mitk::CrashDumpFacility::WriteSettings(const CrashDumpSettings& settings)
{
  return WriteCrashDumpSettings(s_State.DatabaseDirectory, settings);
}

bool mitk::CrashDumpFacility::SupportsSnapshots() noexcept
{
#if defined(__APPLE__)
  return false; // see CaptureSnapshot()
#else
  return true;
#endif
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
  // (excluded from the surfacable set) can never mask a real crash dump, nor
  // a provisional snapshot of this session mask a hard kill that follows it.
  const auto dumps = WithoutProvisionalSnapshotsOfThisSession(ScanCrashDumps(
    s_State.DatabaseDirectory, { kSnapshotsSubdir, kCrashpadStagingSubdir }));
  if (!dumps.empty())
    WriteLastAcknowledgedTime(s_State.DatabaseDirectory, dumps.front().LastWriteTime);
}

std::vector<mitk::CrashDumpInfo> mitk::CrashDumpFacility::ListDumps()
{
  return WithRunInfo(WithoutProvisionalSnapshotsOfThisSession(ScanCrashDumps(
    s_State.DatabaseDirectory, { kSnapshotsSubdir, kCrashpadStagingSubdir })));
}

std::vector<mitk::CrashDumpInfo> mitk::CrashDumpFacility::ListUnacknowledgedDumps()
{
  return WithRunInfo(WithoutProvisionalSnapshotsOfThisSession(ScanUnacknowledgedCrashDumps(
    s_State.DatabaseDirectory, { kSnapshotsSubdir, kCrashpadStagingSubdir })));
}

std::vector<mitk::CrashDumpInfo> mitk::CrashDumpFacility::ListAllDumps()
{
  return WithRunInfo(WithoutProvisionalSnapshotsOfThisSession(ScanCrashDumps(
    s_State.DatabaseDirectory, { kCrashpadStagingSubdir })));
}

std::vector<mitk::CrashDumpInfo> mitk::CrashDumpFacility::ListProvisionalSnapshotsOfThisSession()
{
  if (s_State.DatabaseDirectory.empty())
    return {};

  const auto provisional = ProvisionalSnapshotsOfThisSession();
  auto dumps = ScanCrashDumps(s_State.DatabaseDirectory / kPendingFreezeSubdir);

  std::erase_if(dumps, [&provisional](const CrashDumpInfo& dump) {
    return provisional.find(dump.Path) == provisional.end();
  });

  return WithRunInfo(dumps);
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
  const bool removed = std::filesystem::remove(dumpPath, error) && !error;

  std::filesystem::remove(GetRunInfoSidecarPath(dumpPath), error);
  RemoveCrashReportResidue(s_State.DatabaseDirectory, dumpPath);

  return removed;
}

std::filesystem::path mitk::CrashDumpFacility::GetDatabaseDirectory()
{
  return s_State.DatabaseDirectory;
}

void mitk::CrashDumpFacility::SetSessionLogFile(const std::filesystem::path& logFile)
{
  std::lock_guard<std::mutex> lock(s_RunInfoMutex);

  if (!s_State.Active || s_State.RunInfoFile.empty())
    return;

  s_State.RunInfo.LogFile = logFile;

  if (!WriteRunInfo(s_State.RunInfoFile, s_State.RunInfo))
    MITK_WARN << "Crash-dump facility: cannot record the session log in '" << s_State.RunInfoFile.string() << "'.";
}

std::optional<std::filesystem::path> mitk::CrashDumpFacility::CaptureSnapshot(
  [[maybe_unused]] SnapshotKind kind)
{
  std::lock_guard<std::mutex> lock(s_SnapshotMutex);

  if (!s_State.Active)
  {
    MITK_WARN << "Cannot capture a diagnostic snapshot: the crash-dump facility is not active.";
    return std::nullopt;
  }

#if defined(__APPLE__)
  // Crashpad's macOS client has no DumpWithoutCrash (macOS drives dumps only
  // through a Mach exception server, on an actual crash), so on-demand and
  // watchdog snapshots cannot be taken there. Hard-crash capture is
  // unaffected.
  MITK_WARN << "On-demand diagnostic snapshots are not available on macOS.";
  return std::nullopt;
#else
  const auto& database = s_State.DatabaseDirectory;

  // DumpWithoutCrash writes into Crashpad's report area; diff it against the
  // set present before the call to find the freshly written file. The facility
  // subdirectories are excluded so a previously filed snapshot is never
  // mistaken for the new one, and the staging directory so a dump the handler
  // has not finished is never taken for a complete one.
  const std::vector<std::filesystem::path> reportArea = {
    kSnapshotsSubdir, kPendingFreezeSubdir, kCrashpadStagingSubdir };

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

  const auto destination = destinationDir /
    (newDump.stem().string() + "-" + CaptureTimestamp() + newDump.extension().string());

  std::error_code renameError;
  std::filesystem::rename(newDump, destination, renameError);

  if (renameError)
  {
    MITK_WARN << "Could not file the diagnostic snapshot under '" << destinationDir.string()
              << "': " << renameError.message();
    std::filesystem::remove(newDump, error); // avoid a stray dump surfacing as a crash
  }
  else
  {
    // The run info lives in the report's attachments, which go with the
    // report below.
    AdoptRunInfoAttachment(database, newDump.stem(), destination);
  }

  // Either way the dump is no longer Crashpad's to hold - filed under its
  // kind, or discarded above - so the report has to be released here, after
  // the discard rather than before it: a report released while its dump is
  // still in the report area would leave that dump behind unrecorded.
  RemoveCrashReportResidue(database, newDump);

  if (renameError)
    return std::nullopt;

  if (SnapshotKind::WatchdogProvisional == kind)
    s_State.ProvisionalSnapshots.push_back(destination);

  PruneCrashDumps(destinationDir, static_cast<std::size_t>(s_State.Settings.MaxDumpsPerKind));

  return destination;
#endif
}

void mitk::CrashDumpFacility::PurgeProvisionalSnapshots()
{
  std::lock_guard<std::mutex> lock(s_SnapshotMutex);

  for (const auto& snapshot : s_State.ProvisionalSnapshots)
  {
    std::error_code error;
    std::filesystem::remove(snapshot, error);
    std::filesystem::remove(GetRunInfoSidecarPath(snapshot), error);
  }

  s_State.ProvisionalSnapshots.clear();
}

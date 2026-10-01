/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCrashDumpFacility_h
#define mitkCrashDumpFacility_h

#include <MitkCrashHandlingExports.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace mitk
{
  /** \brief Area a dump was filed in, which is what it means to the user. */
  enum class DumpKind
  {
    Crash,                  /**< Hard crash written by the handler. */
    UnresponsiveTerminated, /**< Watchdog snapshot of a freeze that was hard-killed. */
    OnDemand                /**< "Capture diagnostics" snapshot of a running session. */
  };

  /** \brief Session context recorded alongside a dump. Fields are empty when unknown. */
  struct MITKCRASHHANDLING_EXPORT CrashRunInfo
  {
    /** e.g. "MITK Workbench <version>". */
    std::string Release;
    std::filesystem::path InstallDirectory;
    /** Empty if the dump was taken before the session's log was opened. */
    std::filesystem::path LogFile;
  };

  /** \brief Metadata of one minidump found in the crash-dump database. */
  struct MITKCRASHHANDLING_EXPORT CrashDumpInfo
  {
    std::filesystem::path Path;
    std::filesystem::file_time_type LastWriteTime;
    std::uintmax_t SizeInBytes = 0;
    DumpKind Kind = DumpKind::Crash;
    /** Empty when the dump carries no run info (older dumps, unsupported platform). */
    std::optional<CrashRunInfo> RunInfo;
  };

  /**
   * \brief Persistent crash-dump settings, applied at the next Initialize().
   *
   * Stored in the database directory rather than in the MITK preferences:
   * Initialize() needs them before Qt and the preference service exist (arming
   * that early is what covers startup crashes), and the file has the scope of
   * the dumps it governs - every install and every instance of the
   * application, including --BlueBerry.newInstance sessions.
   */
  struct MITKCRASHHANDLING_EXPORT CrashDumpSettings
  {
    static constexpr int MinRetention = 1;
    /** Matches the number of session logs mitk::LogBackend retains, so that a
     *  kept dump's log can still exist. Change both together. */
    static constexpr int MaxRetention = 10;
    static constexpr int MinWatchdogTimeoutSeconds = 10;
    static constexpr int MaxWatchdogTimeoutSeconds = 3600;

    bool Enabled = true;
    /** Retention per dump area; clamped to [MinRetention, MaxRetention]. */
    int MaxDumpsPerKind = MaxRetention;
    /** UI-freeze watchdog timeout; 0 disables it, otherwise clamped to
     *  [MinWatchdogTimeoutSeconds, MaxWatchdogTimeoutSeconds]. */
    int WatchdogTimeoutSeconds = 0;
  };

  /**
   * \brief Provenance of a non-fatal snapshot, which governs whether it is
   *        surfaced on the next start and how long it is retained.
   *
   * OnDemand: the user pressed "Capture diagnostics"; shown once, kept, and
   * never re-surfaced by the next-start dialog. WatchdogProvisional: written
   * by the UI-freeze watchdog; purged when the freeze recovers or on clean
   * shutdown, so only a hard-killed freeze leaves one behind (and only those
   * survivors are surfaced).
   */
  enum class SnapshotKind
  {
    OnDemand,
    WatchdogProvisional
  };

  /**
   * \brief Process-wide crash-dump facility (out-of-process Crashpad via
   *        sentry-native, local-only).
   *
   * Initialize() arms an external crashpad_handler process that writes a
   * minidump into the configured database directory when this process
   * crashes hard (access violation, abort, stack overflow). Nothing is ever
   * uploaded: no DSN is compiled in, no API sets one, and sentry's transport
   * is compiled out, so the handler has no upload endpoint.
   *
   * Contract notes:
   * - Static-only; must be initialized at most once per process, as early
   *   as possible. MitkCrashHandling is the only module that may link the
   *   underlying (static) sentry/crashpad libraries; a second linking
   *   target would duplicate process-global handler state.
   * - Initialize() never throws and never aborts application startup: a
   *   diagnostics facility must not break the application it diagnoses.
   *   Failure is reported via return value and MITK_WARN. This is a
   *   deliberate exception to the throw-on-unresolvable-input rule.
   */
  class MITKCRASHHANDLING_EXPORT CrashDumpFacility
  {
  public:
    struct Config
    {
      /** Crash-dump database directory; created if missing. */
      std::filesystem::path DatabaseDirectory;
      /** Reported as release annotation, e.g. "MITK Workbench <version>". */
      std::string ApplicationName;
      std::string ApplicationVersion;
      /** False when crash dumps are disabled for this session (environment
       *  variable or command-line flag): the database directory and settings
       *  are still loaded so existing dumps can be listed and handled, but the
       *  handler is not armed. */
      bool Arm = true;
    };

    CrashDumpFacility() = delete;

    /** \brief Load the settings and arm the handler.
     *
     *  Reads the settings file first and arms only if Config::Arm and the
     *  settings enable it. Returns whether the handler is armed: false with
     *  MITK_INFO when arming is disabled, false with MITK_WARN when
     *  crashpad_handler next to the current executable is missing or the
     *  backend failed to start. Caches the crashed-last-run flag (see
     *  CrashedLastRun()) and prunes each dump area to the newest
     *  MaxDumpsPerKind dumps (the just-crashed dump is by definition the
     *  newest and always survives). The database directory is recorded
     *  whenever it exists or could be created, armed or not, so the query
     *  methods below remain usable. */
    static bool Initialize(const Config& config) noexcept;

    /** \brief Orderly teardown (normal shutdown only; never in a crash path). */
    static void Shutdown() noexcept;

    static bool IsActive() noexcept;

    /** \brief Settings this session runs with, as loaded (and clamped) by
     *  Initialize(). Defaults when the facility was never initialized. */
    static CrashDumpSettings GetSettings() noexcept;

    /** \brief Settings as currently stored, i.e. what the next start will
     *  use. Defaults when there is no database directory or no settings file. */
    static CrashDumpSettings ReadSettings();

    /** \brief Persist \p settings for the next start; values are clamped.
     *  Does not change the running session. Returns false if the file could
     *  not be written (e.g. no database directory). */
    static bool WriteSettings(const CrashDumpSettings& settings);

    /** \brief False where non-fatal snapshots (on-demand, watchdog) cannot be
     *  taken (macOS). */
    static bool SupportsSnapshots() noexcept;

    /** \brief True when the previous run of this database's application
     *  ended in a crash. Value is read once during Initialize() and cached,
     *  so later runs sharing the database cannot mask it mid-session.
     *  Corroborating signal only; the robust next-launch trigger is
     *  ListUnacknowledgedDumps(). */
    static bool CrashedLastRun() noexcept;

    /** \brief Acknowledge the currently present dumps (dialog "seen"): dumps
     *  present now will not be reported by ListUnacknowledgedDumps() again,
     *  and the crashed-last-run marker is cleared. */
    static void ClearCrashedLastRun();

    /** \brief The surfacable dumps the next-start dialog shows, newest
     *  first: crash dumps plus hard-killed UI-freeze survivors. This
     *  session's own provisional snapshots are excluded. On-demand
     *  snapshots (see CaptureSnapshot) are held in a separate area and
     *  excluded. *.dmp files only; internal run/metadata directories are not
     *  part of the contract. Returns an empty list when the database
     *  directory does not exist. */
    static std::vector<CrashDumpInfo> ListDumps();

    /** \brief The subset of ListDumps() newer than the last
     *  acknowledgment (ClearCrashedLastRun()) - the next-launch trigger:
     *  a dialog is warranted exactly when this is non-empty. */
    static std::vector<CrashDumpInfo> ListUnacknowledgedDumps();

    /** \brief Every dump the user can handle, all kinds, newest first, with
     *  their run info. Excludes this session's provisional watchdog
     *  snapshots: they are deleted automatically when the freeze recovers or
     *  on clean shutdown, so they are not the user's to handle. */
    static std::vector<CrashDumpInfo> ListAllDumps();

    /** \brief This session's provisional watchdog snapshots, newest first
     *  (the complement of what ListAllDumps() leaves out). */
    static std::vector<CrashDumpInfo> ListProvisionalSnapshotsOfThisSession();

    /** \brief The non-fatal snapshots currently filed under \p kind, newest
     *  first. Complements ListDumps(), which never reports the on-demand
     *  area; provisional (watchdog) snapshots of earlier sessions appear in
     *  both. Run info is not loaded. Returns an empty list when the area does
     *  not exist. */
    static std::vector<CrashDumpInfo> ListSnapshots(SnapshotKind kind);

    /** \brief Remove one dump file, along with its run info and the crash
     *  report that owned it: its metadata and anything the handler attached
     *  to it. Returns
     *  whether the dump file itself went; the report is released either way
     *  and best effort, so a false return does not mean nothing changed. */
    static bool DeleteDump(const std::filesystem::path& dumpPath);

    static std::filesystem::path GetDatabaseDirectory();

    /** \brief Record the session's log file in the run info attached to
     *  dumps this process produces from now on. Called once the log has been
     *  opened, which happens after Initialize(). No-op when not armed. */
    static void SetSessionLogFile(const std::filesystem::path& logFile);

    /** \brief Write a minidump of the running process without crashing it
     *  (Crashpad DumpWithoutCrash) and file it under \p kind. Returns the
     *  new dump's path, or nullopt when the facility is inactive or the
     *  freshly written dump cannot be located. Safe to call from any thread;
     *  captures the calling thread's context. */
    static std::optional<std::filesystem::path> CaptureSnapshot(
      SnapshotKind kind = SnapshotKind::OnDemand);

    /** \brief Delete all provisional (watchdog) snapshots. Called when a UI
     *  freeze recovers and on clean shutdown, so only a hard-killed freeze
     *  leaves a dump behind. */
    static void PurgeProvisionalSnapshots();
  };
}

#endif

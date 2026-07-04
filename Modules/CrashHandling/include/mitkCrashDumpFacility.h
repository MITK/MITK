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
#include <string>
#include <vector>

namespace mitk
{
  /** \brief Metadata of one minidump found in the crash-dump database. */
  struct MITKCRASHHANDLING_EXPORT CrashDumpInfo
  {
    std::filesystem::path Path;
    std::filesystem::file_time_type LastWriteTime;
    std::uintmax_t SizeInBytes = 0;
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
    };

    CrashDumpFacility() = delete;

    /** \brief Arm the handler. False (with MITK_WARN) if crashpad_handler
     *  next to the current executable is missing or the backend failed to
     *  start. Caches the crashed-last-run flag (see CrashedLastRun()) and
     *  prunes the database to the newest kMaxRetainedDumps dumps (the
     *  just-crashed dump is by definition the newest and always survives).
     *  The database directory is recorded even when arming fails, so the
     *  query methods below remain usable. */
    static bool Initialize(const Config& config) noexcept;

    /** \brief Orderly teardown (normal shutdown only; never in a crash path). */
    static void Shutdown() noexcept;

    static bool IsActive() noexcept;

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
     *  first. *.dmp files only; internal run/metadata directories are not
     *  part of the contract. Returns an empty list when the database
     *  directory does not exist. */
    static std::vector<CrashDumpInfo> ListDumps();

    /** \brief The subset of ListDumps() newer than the last
     *  acknowledgment (ClearCrashedLastRun()) - the next-launch trigger:
     *  a dialog is warranted exactly when this is non-empty. */
    static std::vector<CrashDumpInfo> ListUnacknowledgedDumps();

    /** \brief Remove one dump file (and, best effort, its metadata). */
    static bool DeleteDump(const std::filesystem::path& dumpPath);

    static std::filesystem::path GetDatabaseDirectory();
  };
}

#endif

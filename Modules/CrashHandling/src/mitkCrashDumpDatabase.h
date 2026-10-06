/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCrashDumpDatabase_h
#define mitkCrashDumpDatabase_h

#include <mitkCrashDumpFacility.h>

#include <cstddef>
#include <filesystem>
#include <optional>
#include <vector>

/**
 * Filesystem-level operations on the crash-dump database directory.
 * Internal to MitkCrashHandling (exported only for the module tests);
 * mitk::CrashDumpFacility is the public interface.
 *
 * All functions are deliberately non-throwing on filesystem errors: they
 * serve a diagnostics facility that must not disturb the application, so
 * a missing directory reads as "no dumps" and failed deletions are
 * reported via return values.
 *
 * \p excludedSubdirs lets a caller skip facility-owned subdirectories by
 * name (e.g. the on-demand-snapshot area, which must not surface in the
 * next-start dialog). A dump is skipped when any component of its path
 * matches one of the given names.
 */
namespace mitk
{
  /** Facility-owned subdirectories of the database. Snapshots are moved out
   *  of Crashpad's report area into one of these so their kind is a
   *  filesystem fact that survives a hard kill. */
  inline const std::filesystem::path OnDemandSnapshotsSubdir = "mitk-snapshots";
  inline const std::filesystem::path PendingFreezeSubdir = "mitk-pending-freeze";

  /** Base name of the run-info file the handler attaches to every report.
   *  Crashpad keeps an attachment's base name, which is how it is found
   *  again under attachments/<report-uuid>/. */
  inline const std::filesystem::path RunInfoAttachmentFileName = "mitk-run-info.json";

  /** \brief Kind of the dump at \p dumpPath, from the area it is filed in.
   *  Snapshot areas are flat, so the immediate parent decides; everything
   *  else belongs to Crashpad's report area. */
  MITKCRASHHANDLING_EXPORT DumpKind ClassifyDump(const std::filesystem::path& dumpPath);

  /** \brief All *.dmp files under \p databaseDirectory (recursive), newest
   *  first; ties broken by path for deterministic order. Kind is set, RunInfo
   *  is not (see LoadRunInfo()). */
  MITKCRASHHANDLING_EXPORT std::vector<CrashDumpInfo> ScanCrashDumps(
    const std::filesystem::path& databaseDirectory,
    const std::vector<std::filesystem::path>& excludedSubdirs = {});

  /** \brief The subset of ScanCrashDumps() newer than the acknowledgment
   *  watermark; everything when no watermark exists. */
  MITKCRASHHANDLING_EXPORT std::vector<CrashDumpInfo> ScanUnacknowledgedCrashDumps(
    const std::filesystem::path& databaseDirectory,
    const std::vector<std::filesystem::path>& excludedSubdirs = {});

  /** \brief Delete the oldest dumps, with their run info and log copies, so
   *  that at most \p maxCount remain. Returns the number of dumps actually deleted. */
  MITKCRASHHANDLING_EXPORT std::size_t PruneCrashDumps(
    const std::filesystem::path& databaseDirectory, std::size_t maxCount,
    const std::vector<std::filesystem::path>& excludedSubdirs = {});

  /** \brief The acknowledgment watermark: dumps at or before this time have
   *  already been shown to the user. Empty if never acknowledged (or the
   *  marker file is unreadable, in which case every dump counts as new -
   *  re-surfacing a seen dump is the safe failure direction). */
  MITKCRASHHANDLING_EXPORT std::optional<std::filesystem::file_time_type>
    ReadLastAcknowledgedTime(const std::filesystem::path& databaseDirectory);

  MITKCRASHHANDLING_EXPORT bool WriteLastAcknowledgedTime(
    const std::filesystem::path& databaseDirectory,
    std::filesystem::file_time_type time);

  /** \brief Release the report that owned \p dumpPath, so the session can
   *  file another one.
   *
   *  MITK takes dumps out of Crashpad's report area but Crashpad's records
   *  still reference them. Leaving those records behind is not just untidy:
   *  the backend claims one report ID for the whole session and refuses to
   *  finalize a second report while the first one's metadata still exists,
   *  so an orphaned record silently prevents any further snapshot for the
   *  life of the process. A session that has already filed a real crash dump
   *  keeps its report ID legitimately taken and can still take no further
   *  snapshot - acceptable, because that process is on its way out.
   *
   *  Removes the record as Crashpad's POSIX database keeps it: the sibling
   *  .meta file and the report's subdirectory of \p databaseDirectory /
   *  attachments, both named after the report UUID. The Windows and macOS
   *  backends store report metadata differently and are unaffected by the
   *  blocking described above.
   *
   *  No-op when \p databaseDirectory is empty or relative, or when the report
   *  UUID derived from \p dumpPath is a dot-name, so neither a caller without
   *  facility state nor an oddly named file can delete anything unintended.
   *  Best effort otherwise: filesystem failures are ignored, in line with the
   *  rest of this header. */
  MITKCRASHHANDLING_EXPORT void RemoveCrashReportResidue(
    const std::filesystem::path& databaseDirectory,
    const std::filesystem::path& dumpPath);

  /** \brief Location of the watermark marker file inside the database. */
  MITKCRASHHANDLING_EXPORT std::filesystem::path GetAcknowledgedMarkerFilePath(
    const std::filesystem::path& databaseDirectory);

  /** \brief Location of the settings file inside the database. */
  MITKCRASHHANDLING_EXPORT std::filesystem::path GetSettingsFilePath(
    const std::filesystem::path& databaseDirectory);

  /** \brief \p settings with every value moved into its documented range.
   *  With \p warn, each corrected value is reported via MITK_WARN. */
  MITKCRASHHANDLING_EXPORT CrashDumpSettings ClampCrashDumpSettings(
    const CrashDumpSettings& settings, bool warn);

  /** \brief The stored settings, clamped. A missing file yields the defaults
   *  silently; an unreadable or malformed file, or a key of the wrong type,
   *  yields the defaults for what could not be read, with MITK_WARN. */
  MITKCRASHHANDLING_EXPORT CrashDumpSettings ReadCrashDumpSettings(
    const std::filesystem::path& databaseDirectory);

  /** \brief Store \p settings (clamped). False if the file could not be
   *  written. */
  MITKCRASHHANDLING_EXPORT bool WriteCrashDumpSettings(
    const std::filesystem::path& databaseDirectory, const CrashDumpSettings& settings);

  /** \brief Where the run info of \p dumpPath is kept: next to it, as
   *  "<dump file name>.json". */
  MITKCRASHHANDLING_EXPORT std::filesystem::path GetRunInfoSidecarPath(
    const std::filesystem::path& dumpPath);

  /** \brief The run info stored in \p file; nullopt if it is missing or
   *  malformed. */
  MITKCRASHHANDLING_EXPORT std::optional<CrashRunInfo> ReadRunInfo(const std::filesystem::path& file);

  /** \brief Store \p runInfo in \p file, replacing it atomically so that a
   *  handler copying the file at capture time never reads a partial one. */
  MITKCRASHHANDLING_EXPORT bool WriteRunInfo(const std::filesystem::path& file, const CrashRunInfo& runInfo);

  /** \brief Give the dump \p dumpPath its sidecar from the run-info
   *  attachment of report \p reportId, unless it already has one. Returns
   *  whether a sidecar exists afterwards. */
  MITKCRASHHANDLING_EXPORT bool AdoptRunInfoAttachment(const std::filesystem::path& databaseDirectory,
    const std::filesystem::path& reportId, const std::filesystem::path& dumpPath);

  /** \brief Where the copy of a dump's session log is kept: next to it, as
   *  "<dump file name>.log". */
  MITKCRASHHANDLING_EXPORT std::filesystem::path GetSessionLogCopyPath(
    const std::filesystem::path& dumpPath);

  /** \brief Keep a copy of \p logFile with the dump \p dumpPath, unless it
   *  already has one. Returns whether a copy exists afterwards.
   *
   *  Log rotation hands the session's log name to the next session of the
   *  same install, so the file found under that name is the dump's own log
   *  only until then. With \p notAfter, a log written later than that is
   *  taken to be such a later session's and not copied. An existing copy is
   *  never replaced. */
  MITKCRASHHANDLING_EXPORT bool KeepSessionLog(const std::filesystem::path& dumpPath,
    const std::filesystem::path& logFile, std::optional<std::filesystem::file_time_type> notAfter);

  /** \brief Fill \p dump's RunInfo and SessionLog from the files kept next
   *  to it, if there are any. */
  MITKCRASHHANDLING_EXPORT void LoadRunInfo(CrashDumpInfo& dump);
}

#endif

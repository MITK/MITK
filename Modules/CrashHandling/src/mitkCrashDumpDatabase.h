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
  /** \brief All *.dmp files under \p databaseDirectory (recursive), newest
   *  first; ties broken by path for deterministic order. */
  MITKCRASHHANDLING_EXPORT std::vector<CrashDumpInfo> ScanCrashDumps(
    const std::filesystem::path& databaseDirectory,
    const std::vector<std::filesystem::path>& excludedSubdirs = {});

  /** \brief The subset of ScanCrashDumps() newer than the acknowledgment
   *  watermark; everything when no watermark exists. */
  MITKCRASHHANDLING_EXPORT std::vector<CrashDumpInfo> ScanUnacknowledgedCrashDumps(
    const std::filesystem::path& databaseDirectory,
    const std::vector<std::filesystem::path>& excludedSubdirs = {});

  /** \brief Delete the oldest dumps so that at most \p maxCount remain.
   *  Returns the number of dumps actually deleted. */
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
}

#endif

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

  /** \brief Location of the watermark marker file inside the database. */
  MITKCRASHHANDLING_EXPORT std::filesystem::path GetAcknowledgedMarkerFilePath(
    const std::filesystem::path& databaseDirectory);
}

#endif

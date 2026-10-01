/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCrashDumpDatabase.h"

#include <mitkLog.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>
#include <type_traits>

namespace
{
  // Live in the database directory next to Crashpad's own bookkeeping;
  // Crashpad ignores files it does not know.
  const std::filesystem::path kAcknowledgedMarkerFileName = "mitk-last-acknowledged";
  const std::filesystem::path kSettingsFileName = "mitk-settings.json";

  // Crashpad's POSIX database layout: one <report-uuid>.meta next to the dump
  // and one attachments/<report-uuid>/ directory under the database root. The
  // Windows backend keeps report metadata in a single database-wide file and
  // the macOS one in extended attributes on the dump, so the .meta half finds
  // nothing there.
  const std::filesystem::path kMetadataExtension = ".meta";
  const std::filesystem::path kAttachmentsSubdir = "attachments";

  bool HasDumpExtension(const std::filesystem::path& path)
  {
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".dmp";
  }

  /** Excluded names are matched against \p path below \p databaseDirectory
   *  only. Matching the whole path would let a component of the database's
   *  own location (an installation under a directory called "new", say)
   *  exclude every dump in it. */
  bool IsExcluded(const std::filesystem::path& path,
    const std::filesystem::path& databaseDirectory,
    const std::vector<std::filesystem::path>& excludedSubdirs)
  {
    if (excludedSubdirs.empty())
      return false;

    for (const auto& component : path.lexically_relative(databaseDirectory))
    {
      if (std::find(excludedSubdirs.begin(), excludedSubdirs.end(), component) != excludedSubdirs.end())
        return true;
    }

    return false;
  }

  // Paths are stored as UTF-8 so that a profile directory with non-ASCII
  // characters survives the round trip regardless of the local code page.
  std::string ToUtf8(const std::filesystem::path& path)
  {
    const auto utf8 = path.u8string();
    return std::string(utf8.begin(), utf8.end());
  }

  std::filesystem::path FromUtf8(const std::string& utf8)
  {
    return std::filesystem::path(std::u8string(utf8.begin(), utf8.end()));
  }

  /** Replaces \p file via a temporary and a rename, so that a concurrent
   *  reader (another instance, or the handler copying an attachment) never
   *  sees a partial file. */
  bool WriteFileAtomically(const std::filesystem::path& file, const std::string& content)
  {
    auto temporary = file;
    temporary += ".tmp";

    {
      std::ofstream stream(temporary, std::ios::trunc);
      stream << content;

      if (!stream.good())
        return false;
    }

    std::error_code error;
    std::filesystem::rename(temporary, file, error);

    if (error)
    {
      std::filesystem::remove(temporary, error);
      return false;
    }

    return true;
  }

  void RemoveSidecar(const std::filesystem::path& dumpPath)
  {
    std::error_code error;
    std::filesystem::remove(mitk::GetRunInfoSidecarPath(dumpPath), error);
  }

  /** Reads \p key into \p value if present with the right type; a present
   *  key of the wrong type keeps the default and warns. */
  template <typename T>
  void ReadSetting(const nlohmann::json& json, const char* key, T& value)
  {
    const auto it = json.find(key);
    if (it == json.end())
      return;

    const bool typeMatches = std::is_same_v<T, bool> ? it->is_boolean() : it->is_number_integer();
    if (!typeMatches)
    {
      MITK_WARN << "Crash-dump settings: ignoring '" << key << "', which has the wrong type; using "
                << value << ".";
      return;
    }

    value = it->get<T>();
  }
}

mitk::DumpKind mitk::ClassifyDump(const std::filesystem::path& dumpPath)
{
  const auto area = dumpPath.parent_path().filename();

  if (area == OnDemandSnapshotsSubdir)
    return DumpKind::OnDemand;

  if (area == PendingFreezeSubdir)
    return DumpKind::UnresponsiveTerminated;

  return DumpKind::Crash;
}

std::vector<mitk::CrashDumpInfo> mitk::ScanCrashDumps(const std::filesystem::path& databaseDirectory,
  const std::vector<std::filesystem::path>& excludedSubdirs)
{
  std::vector<CrashDumpInfo> dumps;

  std::error_code error;
  std::filesystem::recursive_directory_iterator it(databaseDirectory,
    std::filesystem::directory_options::skip_permission_denied, error);

  if (error)
    return dumps;

  // recursive_directory_iterator::operator++ throws on a mid-iteration
  // filesystem error (e.g. a dump removed by a concurrent prune). This
  // facility must not disturb the application it diagnoses, so treat such an
  // error as end-of-scan and return whatever was collected so far.
  try
  {
    for (const auto& entry : it)
    {
      if (!entry.is_regular_file(error) || !HasDumpExtension(entry.path()))
        continue;

      if (IsExcluded(entry.path(), databaseDirectory, excludedSubdirs))
        continue;

      const auto lastWriteTime = entry.last_write_time(error);
      if (error)
        continue;

      const auto size = entry.file_size(error);
      if (error)
        continue;

      dumps.push_back({ entry.path(), lastWriteTime, size, ClassifyDump(entry.path()), std::nullopt });
    }
  }
  catch (const std::filesystem::filesystem_error&)
  {
  }

  std::sort(dumps.begin(), dumps.end(), [](const CrashDumpInfo& lhs, const CrashDumpInfo& rhs) {
    return std::tie(rhs.LastWriteTime, rhs.Path) < std::tie(lhs.LastWriteTime, lhs.Path);
  });

  return dumps;
}

std::vector<mitk::CrashDumpInfo> mitk::ScanUnacknowledgedCrashDumps(
  const std::filesystem::path& databaseDirectory,
  const std::vector<std::filesystem::path>& excludedSubdirs)
{
  auto dumps = ScanCrashDumps(databaseDirectory, excludedSubdirs);

  const auto acknowledged = ReadLastAcknowledgedTime(databaseDirectory);
  if (acknowledged.has_value())
  {
    std::erase_if(dumps, [&acknowledged](const CrashDumpInfo& dump) {
      return dump.LastWriteTime <= *acknowledged;
    });
  }

  return dumps;
}

std::size_t mitk::PruneCrashDumps(const std::filesystem::path& databaseDirectory, std::size_t maxCount,
  const std::vector<std::filesystem::path>& excludedSubdirs)
{
  const auto dumps = ScanCrashDumps(databaseDirectory, excludedSubdirs);

  std::size_t deleted = 0;

  for (std::size_t i = maxCount; i < dumps.size(); ++i)
  {
    std::error_code error;
    if (std::filesystem::remove(dumps[i].Path, error) && !error)
    {
      RemoveSidecar(dumps[i].Path);
      ++deleted;
    }
  }

  return deleted;
}

void mitk::RemoveCrashReportResidue(const std::filesystem::path& databaseDirectory,
  const std::filesystem::path& dumpPath)
{
  const auto reportId = dumpPath.stem();

  // "." and ".." are legal stems (a file called "..dmp" yields the former),
  // and either would make the remove_all below escape the report's own
  // directory - into the attachments root, or the database itself.
  if (!databaseDirectory.is_absolute() || reportId.empty() || reportId == "." || reportId == "..")
    return;

  std::error_code error;
  std::filesystem::remove(std::filesystem::path(dumpPath).replace_extension(kMetadataExtension), error);
  std::filesystem::remove_all(databaseDirectory / kAttachmentsSubdir / reportId, error);
}

std::filesystem::path mitk::GetAcknowledgedMarkerFilePath(const std::filesystem::path& databaseDirectory)
{
  return databaseDirectory / kAcknowledgedMarkerFileName;
}

std::optional<std::filesystem::file_time_type> mitk::ReadLastAcknowledgedTime(
  const std::filesystem::path& databaseDirectory)
{
  std::ifstream file(GetAcknowledgedMarkerFilePath(databaseDirectory));

  // file_time_type::rep is __int128 on some standard libraries (libc++), for
  // which the stream operators have no overload. The value is nanoseconds
  // since the epoch and fits a 64-bit integer for any realistic date, so
  // round-trip it as long long.
  long long ticks = 0;
  if (!(file >> ticks))
    return std::nullopt;

  return std::filesystem::file_time_type(std::filesystem::file_time_type::duration(ticks));
}

bool mitk::WriteLastAcknowledgedTime(const std::filesystem::path& databaseDirectory,
  std::filesystem::file_time_type time)
{
  std::error_code error;
  std::filesystem::create_directories(databaseDirectory, error);

  std::ofstream file(GetAcknowledgedMarkerFilePath(databaseDirectory), std::ios::trunc);
  file << static_cast<long long>(time.time_since_epoch().count());

  return file.good();
}

std::filesystem::path mitk::GetSettingsFilePath(const std::filesystem::path& databaseDirectory)
{
  return databaseDirectory / kSettingsFileName;
}

mitk::CrashDumpSettings mitk::ClampCrashDumpSettings(const CrashDumpSettings& settings, bool warn)
{
  auto clamped = settings;

  clamped.MaxDumpsPerKind = std::clamp(settings.MaxDumpsPerKind,
    CrashDumpSettings::MinRetention, CrashDumpSettings::MaxRetention);

  if (settings.WatchdogTimeoutSeconds <= 0)
  {
    clamped.WatchdogTimeoutSeconds = 0;
  }
  else
  {
    clamped.WatchdogTimeoutSeconds = std::clamp(settings.WatchdogTimeoutSeconds,
      CrashDumpSettings::MinWatchdogTimeoutSeconds, CrashDumpSettings::MaxWatchdogTimeoutSeconds);
  }

  if (warn && clamped.MaxDumpsPerKind != settings.MaxDumpsPerKind)
  {
    MITK_WARN << "Crash-dump settings: maxDumpsPerKind " << settings.MaxDumpsPerKind
              << " is out of range, using " << clamped.MaxDumpsPerKind << ".";
  }

  if (warn && clamped.WatchdogTimeoutSeconds != settings.WatchdogTimeoutSeconds)
  {
    MITK_WARN << "Crash-dump settings: watchdogTimeoutSeconds " << settings.WatchdogTimeoutSeconds
              << " is out of range, using " << clamped.WatchdogTimeoutSeconds << ".";
  }

  return clamped;
}

mitk::CrashDumpSettings mitk::ReadCrashDumpSettings(const std::filesystem::path& databaseDirectory)
{
  CrashDumpSettings settings;

  const auto path = GetSettingsFilePath(databaseDirectory);

  std::error_code error;
  if (databaseDirectory.empty() || !std::filesystem::exists(path, error))
    return settings;

  std::ifstream file(path);
  const auto json = nlohmann::json::parse(file, nullptr, false);

  if (json.is_discarded() || !json.is_object())
  {
    MITK_WARN << "Crash-dump settings file '" << path.string() << "' is unreadable or malformed; using defaults.";
    return settings;
  }

  ReadSetting(json, "enabled", settings.Enabled);
  ReadSetting(json, "maxDumpsPerKind", settings.MaxDumpsPerKind);
  ReadSetting(json, "watchdogTimeoutSeconds", settings.WatchdogTimeoutSeconds);

  return ClampCrashDumpSettings(settings, true);
}

bool mitk::WriteCrashDumpSettings(const std::filesystem::path& databaseDirectory,
  const CrashDumpSettings& settings)
{
  if (databaseDirectory.empty())
    return false;

  const auto clamped = ClampCrashDumpSettings(settings, false);

  const nlohmann::json json = {
    { "enabled", clamped.Enabled },
    { "maxDumpsPerKind", clamped.MaxDumpsPerKind },
    { "watchdogTimeoutSeconds", clamped.WatchdogTimeoutSeconds }
  };

  std::error_code error;
  std::filesystem::create_directories(databaseDirectory, error);

  // A start reading a half-written file would fall back to the defaults and
  // could record dumps the user has just switched off.
  return WriteFileAtomically(GetSettingsFilePath(databaseDirectory), json.dump(2) + '\n');
}

std::filesystem::path mitk::GetRunInfoSidecarPath(const std::filesystem::path& dumpPath)
{
  return std::filesystem::path(dumpPath) += ".json";
}

std::optional<mitk::CrashRunInfo> mitk::ReadRunInfo(const std::filesystem::path& file)
{
  std::ifstream stream(file);
  if (!stream)
    return std::nullopt;

  const auto json = nlohmann::json::parse(stream, nullptr, false);
  if (json.is_discarded() || !json.is_object())
    return std::nullopt;

  CrashRunInfo runInfo;
  runInfo.Release = json.value("release", std::string());
  runInfo.InstallDirectory = FromUtf8(json.value("installDirectory", std::string()));
  runInfo.LogFile = FromUtf8(json.value("logFile", std::string()));

  return runInfo;
}

bool mitk::WriteRunInfo(const std::filesystem::path& file, const CrashRunInfo& runInfo)
{
  const nlohmann::json json = {
    { "release", runInfo.Release },
    { "installDirectory", ToUtf8(runInfo.InstallDirectory) },
    { "logFile", ToUtf8(runInfo.LogFile) }
  };

  return WriteFileAtomically(file, json.dump(2) + '\n');
}

bool mitk::AdoptRunInfoAttachment(const std::filesystem::path& databaseDirectory,
  const std::filesystem::path& reportId, const std::filesystem::path& dumpPath)
{
  const auto sidecar = GetRunInfoSidecarPath(dumpPath);

  std::error_code error;
  if (std::filesystem::exists(sidecar, error))
    return true;

  if (databaseDirectory.empty() || reportId.empty())
    return false;

  const auto attachment = databaseDirectory / kAttachmentsSubdir / reportId / RunInfoAttachmentFileName;
  if (!std::filesystem::exists(attachment, error))
    return false;

  return std::filesystem::copy_file(attachment, sidecar, error) && !error;
}

void mitk::LoadRunInfo(CrashDumpInfo& dump)
{
  dump.RunInfo = ReadRunInfo(GetRunInfoSidecarPath(dump.Path));
}

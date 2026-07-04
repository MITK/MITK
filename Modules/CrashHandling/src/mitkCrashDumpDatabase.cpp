/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCrashDumpDatabase.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>

namespace
{
  // Lives in the database directory next to Crashpad's own bookkeeping;
  // Crashpad ignores files it does not know.
  const std::filesystem::path kAcknowledgedMarkerFileName = "mitk-last-acknowledged";

  bool HasDumpExtension(const std::filesystem::path& path)
  {
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".dmp";
  }
}

std::vector<mitk::CrashDumpInfo> mitk::ScanCrashDumps(const std::filesystem::path& databaseDirectory)
{
  std::vector<CrashDumpInfo> dumps;

  std::error_code error;
  std::filesystem::recursive_directory_iterator it(databaseDirectory,
    std::filesystem::directory_options::skip_permission_denied, error);

  if (error)
    return dumps;

  for (const auto& entry : it)
  {
    if (!entry.is_regular_file(error) || !HasDumpExtension(entry.path()))
      continue;

    const auto lastWriteTime = entry.last_write_time(error);
    if (error)
      continue;

    const auto size = entry.file_size(error);
    if (error)
      continue;

    dumps.push_back({ entry.path(), lastWriteTime, size });
  }

  std::sort(dumps.begin(), dumps.end(), [](const CrashDumpInfo& lhs, const CrashDumpInfo& rhs) {
    return std::tie(rhs.LastWriteTime, rhs.Path) < std::tie(lhs.LastWriteTime, lhs.Path);
  });

  return dumps;
}

std::vector<mitk::CrashDumpInfo> mitk::ScanUnacknowledgedCrashDumps(
  const std::filesystem::path& databaseDirectory)
{
  auto dumps = ScanCrashDumps(databaseDirectory);

  const auto acknowledged = ReadLastAcknowledgedTime(databaseDirectory);
  if (acknowledged.has_value())
  {
    std::erase_if(dumps, [&acknowledged](const CrashDumpInfo& dump) {
      return dump.LastWriteTime <= *acknowledged;
    });
  }

  return dumps;
}

std::size_t mitk::PruneCrashDumps(const std::filesystem::path& databaseDirectory, std::size_t maxCount)
{
  const auto dumps = ScanCrashDumps(databaseDirectory);

  std::size_t deleted = 0;

  for (std::size_t i = maxCount; i < dumps.size(); ++i)
  {
    std::error_code error;
    if (std::filesystem::remove(dumps[i].Path, error) && !error)
      ++deleted;
  }

  return deleted;
}

std::filesystem::path mitk::GetAcknowledgedMarkerFilePath(const std::filesystem::path& databaseDirectory)
{
  return databaseDirectory / kAcknowledgedMarkerFileName;
}

std::optional<std::filesystem::file_time_type> mitk::ReadLastAcknowledgedTime(
  const std::filesystem::path& databaseDirectory)
{
  std::ifstream file(GetAcknowledgedMarkerFilePath(databaseDirectory));

  std::filesystem::file_time_type::rep ticks = 0;
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
  file << time.time_since_epoch().count();

  return file.good();
}

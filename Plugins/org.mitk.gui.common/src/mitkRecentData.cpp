/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRecentData.h"

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>

#include <QDir>
#include <QFileInfo>

#include <string>

namespace
{
  constexpr int MAX_ENTRIES = 7;

  // The default file systems of Windows and macOS ignore case, so differently
  // cased spellings of a path name the same file there.
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
  constexpr auto PATH_CASE_SENSITIVITY = Qt::CaseInsensitive;
#else
  constexpr auto PATH_CASE_SENSITIVITY = Qt::CaseSensitive;
#endif

  mitk::IPreferences* GetPreferences(mitk::RecentData::Kind kind)
  {
    const std::string node = kind == mitk::RecentData::Kind::Project
      ? "org.mitk.gui.common/recent projects"
      : "org.mitk.gui.common/recent files";

    return mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node(node);
  }

  mitk::RecentData::Kind GetKind(const QString& path)
  {
    return path.endsWith(".mitk", Qt::CaseInsensitive) || path.endsWith(".mitkscene.json", Qt::CaseInsensitive)
      ? mitk::RecentData::Kind::Project
      : mitk::RecentData::Kind::File;
  }

  bool RemovePath(QStringList& paths, const QString& path)
  {
    return paths.removeIf([&path](const QString& p) {
      return p.compare(path, PATH_CASE_SENSITIVITY) == 0;
    }) > 0;
  }

  // Returns whether the stored paths changed. Nothing is written to disk
  // before Commit().
  bool Store(mitk::RecentData::Kind kind, QStringList paths)
  {
    paths = paths.mid(0, MAX_ENTRIES);

    if (paths == mitk::RecentData::Get(kind))
      return false;

    auto* prefs = GetPreferences(kind);
    prefs->Clear();

    for (int i = 0; i < paths.size(); ++i)
      prefs->Put(std::to_string(i), paths[i].toStdString());

    return true;
  }

  void Commit()
  {
    // Flushing any node writes all preferences, so this covers both lists.
    GetPreferences(mitk::RecentData::Kind::File)->Flush();
    mitk::RecentData::OnChanged().Send();
  }
}

QStringList mitk::RecentData::Get(Kind kind)
{
  const auto* prefs = GetPreferences(kind);
  QStringList paths;

  for (int i = 0; i < MAX_ENTRIES; ++i)
  {
    const auto path = prefs->Get(std::to_string(i), "");

    if (!path.empty())
      paths.append(QString::fromStdString(path));
  }

  return paths;
}

void mitk::RecentData::Add(const QStringList& paths)
{
  if (paths.isEmpty())
    return;

  auto projects = Get(Kind::Project);
  auto files = Get(Kind::File);

  // Walk backwards so that the first given path ends up in front.
  for (auto it = paths.crbegin(); it != paths.crend(); ++it)
  {
    if (it->isEmpty())
      continue;

    const auto path = QDir::cleanPath(QFileInfo(*it).absoluteFilePath());
    auto& list = GetKind(path) == Kind::Project ? projects : files;

    RemovePath(list, path);
    list.prepend(path);
  }

  const bool areProjectsChanged = Store(Kind::Project, projects);
  const bool areFilesChanged = Store(Kind::File, files);

  if (areProjectsChanged || areFilesChanged)
    Commit();
}

void mitk::RecentData::Remove(const QString& path)
{
  bool isChanged = false;

  for (const auto kind : { Kind::Project, Kind::File })
  {
    auto list = Get(kind);

    if (RemovePath(list, path))
      isChanged = Store(kind, list) || isChanged;
  }

  if (isChanged)
    Commit();
}

void mitk::RecentData::Clear(Kind kind)
{
  if (Store(kind, {}))
    Commit();
}

const mitk::Message<>& mitk::RecentData::OnChanged()
{
  static Message<> message;
  return message;
}

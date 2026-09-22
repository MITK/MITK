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

  void Store(mitk::RecentData::Kind kind, const QStringList& paths)
  {
    auto* prefs = GetPreferences(kind);
    prefs->Clear();

    for (int i = 0; i < paths.size() && i < MAX_ENTRIES; ++i)
      prefs->Put(std::to_string(i), paths[i].toStdString());

    prefs->Flush();
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

    list.removeAll(path);
    list.prepend(path);
  }

  Store(Kind::Project, projects);
  Store(Kind::File, files);
}

void mitk::RecentData::Remove(const QString& path)
{
  for (const auto kind : { Kind::Project, Kind::File })
  {
    auto list = Get(kind);

    if (list.removeAll(path) > 0)
      Store(kind, list);
  }
}

void mitk::RecentData::Clear(Kind kind)
{
  Store(kind, {});
}

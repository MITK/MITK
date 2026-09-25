/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkToolBarPresets.h"

#include "QmitkApplicationConstants.h"

#include <berryIViewRegistry.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchWindow.h>
#include <berryPlatformUI.h>

#include <mitkCoreServices.h>
#include <mitkExceptionMacro.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkLog.h>

#include <QAction>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QToolBar>

#include <nlohmann/json.hpp>

#include <algorithm>

QT_BEGIN_NAMESPACE

  // Make nlohmann_json understand QString for deserialization.
  void from_json(const nlohmann::json& j, QString& result)
  {
    result = QString::fromStdString(j.get<std::string>());
  }

QT_END_NAMESPACE

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node(QmitkApplicationConstants::TOOL_BARS_PREFERENCES);
  }

  struct PresetFile
  {
    QmitkToolBarPreset Preset;
    QString Ancestor;
  };

  /* Presets are stored in an internal file format:
   *
   *   {
   *     "name": "Mandatory name that is displayed in the UI",
   *     "info": "Optional description in Qt's <b>rich text format</b>",
   *     "ancestor": "Optional name of another preset whose categories are inherited",
   *     "categories": [
   *       "Mandatory list of categories (resp. tool bars) to show"
   *     ]
   *   }
   */
  PresetFile ReadPresetFile(const QString& fileName)
  {
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      mitkThrow() << "Couldn't open \"" << fileName.toStdString() << "\"!";

    const auto j = nlohmann::json::parse(QTextStream(&file).readAll().toStdString(), nullptr, false, true);

    if (j.is_discarded())
      mitkThrow() << "Couldn't parse \"" << fileName.toStdString() << "\"!";

    PresetFile presetFile;

    try
    {
      presetFile.Preset.Name = j.at("name").get<QString>();
      presetFile.Preset.Info = j.value("info", QString());
      presetFile.Preset.Categories = j.at("categories").get<QStringList>();
      presetFile.Ancestor = j.value("ancestor", QString());
    }
    catch (const nlohmann::json::exception& e)
    {
      mitkThrow() << "Invalid preset \"" << fileName.toStdString() << "\": " << e.what();
    }

    return presetFile;
  }

  bool ApplyToToolBar(const QList<QToolBar*>& toolBars, const QString& category, bool isVisible, bool showCategory)
  {
    auto it = std::find_if(toolBars.cbegin(), toolBars.cend(), [&category](const QToolBar* toolBar) {
      return toolBar->objectName() == category;
    });

    if (it == toolBars.cend())
      return false;

    auto* toolBar = *it;
    toolBar->setVisible(isVisible);

    for (auto* action : toolBar->actions())
    {
      if (action->objectName() == "category")
      {
        action->setVisible(showCategory);
        break;
      }
    }

    return true;
  }
}

std::vector<QmitkToolBarPreset> QmitkToolBarPresets::Load()
{
  std::vector<PresetFile> presetFiles;
  QDirIterator it(":/org_mitk_presets");

  while (it.hasNext())
    presetFiles.push_back(ReadPresetFile(it.next()));

  std::vector<QmitkToolBarPreset> presets;

  for (const auto& presetFile : presetFiles)
  {
    auto preset = presetFile.Preset;
    QStringList lineage = { preset.Name };

    for (auto ancestor = presetFile.Ancestor; !ancestor.isEmpty();)
    {
      if (lineage.contains(ancestor))
      {
        mitkThrow() << "Preset \"" << preset.Name.toStdString() << "\" cannot inherit from \""
                    << ancestor.toStdString() << "\" because it would inherit from itself.";
      }

      auto ancestorFile = std::find_if(presetFiles.cbegin(), presetFiles.cend(), [&ancestor](const PresetFile& p) {
        return p.Preset.Name == ancestor;
      });

      if (ancestorFile == presetFiles.cend())
      {
        mitkThrow() << "Preset \"" << preset.Name.toStdString() << "\" cannot inherit from \""
                    << ancestor.toStdString() << "\" because it couldn't be found.";
      }

      preset.Categories.append(ancestorFile->Preset.Categories);
      lineage.append(ancestor);
      ancestor = ancestorFile->Ancestor;
    }

    preset.Categories.removeDuplicates();
    presets.push_back(preset);
  }

  std::sort(presets.begin(), presets.end(), [](const QmitkToolBarPreset& lhs, const QmitkToolBarPreset& rhs) {
    return lhs.Name < rhs.Name;
  });

  return presets;
}

QStringList QmitkToolBarPresets::GetCategories()
{
  return berry::PlatformUI::GetWorkbench()->GetViewRegistry()->GetViewsByCategory().uniqueKeys();
}

QStringList QmitkToolBarPresets::GetVisibleCategories()
{
  const auto* prefs = GetPreferences();
  QStringList visibleCategories;

  for (const auto& category : GetCategories())
  {
    if (prefs->GetBool(category.toStdString(), true))
      visibleCategories.append(category);
  }

  return visibleCategories;
}

void QmitkToolBarPresets::SetVisibleCategories(const QStringList& categories)
{
  auto* prefs = GetPreferences();

  for (const auto& category : GetCategories())
    prefs->PutBool(category.toStdString(), categories.contains(category));

  prefs->Flush();

  ApplyToWorkbench();
}

void QmitkToolBarPresets::ApplyToWorkbench()
{
  const auto* prefs = GetPreferences();
  const bool showCategories = prefs->GetBool(QmitkApplicationConstants::TOOL_BARS_SHOW_CATEGORIES, true);
  const auto categories = GetCategories();

  for (const auto& window : berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows())
  {
    const auto toolBars = window->GetToolBars();

    for (const auto& category : categories)
    {
      if (!ApplyToToolBar(toolBars, category, prefs->GetBool(category.toStdString(), true), showCategories))
        MITK_WARN << "Could not find tool bar for category \"" << category.toStdString() << "\" to set its visibility!";
    }
  }
}

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
#include <internal/berryQtShowViewAction.h>

#include <mitkCoreServices.h>
#include <mitkExceptionMacro.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkLog.h>

#include <QDirIterator>
#include <QFile>
#include <QMainWindow>
#include <QMultiMap>
#include <QTextStream>
#include <QToolBar>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <utility>

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

  using CategoryLabel = QmitkCategoryToolBar::CategoryLabel;

  const std::array<std::pair<CategoryLabel, std::string>, 3> CATEGORY_LABEL_VALUES = {{
    { CategoryLabel::Hidden, "hidden" },
    { CategoryLabel::AboveButtons, "above buttons" },
    { CategoryLabel::OnHover, "on hover" }
  }};

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

  qsizetype GetCategoryRank(const QString& category)
  {
    const QStringList leading = {
      "Data",
      "Visualization",
      "Segmentation",
      "Processing",
      "Quantification",
      "Registration",
      "Model Fitting",
      "PET"
    };

    const QStringList trailing = {
      "Utilities",
      "Help"
    };

    if (const auto index = leading.indexOf(category); index != -1)
      return index;

    if (const auto index = trailing.indexOf(category); index != -1)
      return leading.size() + 1 + index;

    return leading.size();
  }

  bool ApplyToToolBar(const QList<QToolBar*>& toolBars, const QString& category, bool isVisible)
  {
    auto it = std::find_if(toolBars.cbegin(), toolBars.cend(), [&category](const QToolBar* toolBar) {
      return toolBar->objectName() == category;
    });

    if (it == toolBars.cend())
      return false;

    (*it)->setVisible(isVisible);

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

void QmitkToolBarPresets::CreateToolBars(berry::IWorkbenchWindow* window, QMainWindow* mainWindow, const QList<berry::IViewDescriptor::Pointer>& views)
{
  const auto* prefs = GetPreferences();
  const auto categoryLabel = GetCategoryLabel();

  QMultiMap<QString, berry::IViewDescriptor::Pointer> viewsByCategory;

  for (const auto& view : views)
  {
    const auto categoryPath = view->GetCategoryPath();
    viewsByCategory.insert(!categoryPath.isEmpty() ? categoryPath.back() : QString(), view);
  }

  auto categories = viewsByCategory.uniqueKeys();
  std::sort(categories.begin(), categories.end(), IsCategoryBefore);

  for (const auto& category : std::as_const(categories))
  {
    auto* toolBar = new QmitkCategoryToolBar(category);
    toolBar->setObjectName(category);
    toolBar->SetCategoryLabel(categoryLabel);
    mainWindow->addToolBar(toolBar);

    toolBar->setVisible(prefs->GetBool(category.toStdString(), true));

    for (const auto& view : viewsByCategory.values(category))
      toolBar->addAction(new berry::QtShowViewAction(berry::IWorkbenchWindow::Pointer(window), view));
  }
}

QStringList QmitkToolBarPresets::GetCategories()
{
  auto categories = berry::PlatformUI::GetWorkbench()->GetViewRegistry()->GetViewsByCategory().uniqueKeys();
  std::sort(categories.begin(), categories.end(), IsCategoryBefore);

  return categories;
}

bool QmitkToolBarPresets::IsCategoryBefore(const QString& lhs, const QString& rhs)
{
  const auto lhsRank = GetCategoryRank(lhs);
  const auto rhsRank = GetCategoryRank(rhs);

  return lhsRank != rhsRank
    ? lhsRank < rhsRank
    : lhs.compare(rhs, Qt::CaseInsensitive) < 0;
}

QmitkCategoryToolBar::CategoryLabel QmitkToolBarPresets::GetCategoryLabel()
{
  const auto* prefs = GetPreferences();
  const auto value = prefs->Get(QmitkApplicationConstants::TOOL_BARS_CATEGORY_LABEL, "");

  auto it = std::find_if(CATEGORY_LABEL_VALUES.cbegin(), CATEGORY_LABEL_VALUES.cend(), [&value](const auto& pair) {
    return pair.second == value;
  });

  if (it != CATEGORY_LABEL_VALUES.cend())
    return it->first;

  // Until a category label is stored, respect if category names were turned
  // off by the former on/off choice.
  return prefs->GetBool(QmitkApplicationConstants::TOOL_BARS_SHOW_CATEGORIES, true)
    ? CategoryLabel::AboveButtons
    : CategoryLabel::Hidden;
}

void QmitkToolBarPresets::SetCategoryLabel(QmitkCategoryToolBar::CategoryLabel categoryLabel)
{
  auto it = std::find_if(CATEGORY_LABEL_VALUES.cbegin(), CATEGORY_LABEL_VALUES.cend(), [categoryLabel](const auto& pair) {
    return pair.first == categoryLabel;
  });

  auto* prefs = GetPreferences();
  prefs->Put(QmitkApplicationConstants::TOOL_BARS_CATEGORY_LABEL, it->second);
  prefs->Flush();
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
  const auto categoryLabel = GetCategoryLabel();
  const auto categories = GetCategories();

  for (const auto& window : berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows())
  {
    const auto toolBars = window->GetToolBars();

    for (auto* toolBar : toolBars)
    {
      if (auto* categoryToolBar = qobject_cast<QmitkCategoryToolBar*>(toolBar); nullptr != categoryToolBar)
        categoryToolBar->SetCategoryLabel(categoryLabel);
    }

    for (const auto& category : categories)
    {
      if (!ApplyToToolBar(toolBars, category, prefs->GetBool(category.toStdString(), true)))
        MITK_WARN << "Could not find tool bar for category \"" << category.toStdString() << "\" to set its visibility!";
    }
  }
}

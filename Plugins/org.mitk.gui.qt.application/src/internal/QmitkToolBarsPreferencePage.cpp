/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkToolBarsPreferencePage.h"
#include <ui_QmitkToolBarsPreferencePage.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <QmitkApplicationConstants.h>

#include <berryPlatformUI.h>

#include <QMainWindow>
#include <QToolBar>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto prefService = mitk::CoreServices::GetPreferencesService();
    return prefService->GetSystemPreferences()->Node(QmitkApplicationConstants::TOOL_BARS_PREFERENCES);
  }

  // Get views as multimap with categories as keys.
  //
  // Exclude views without category and categories that contain a literal '.', e.g.
  // "org.blueberry.ui" or "org.mitk.views.general", as they typically do not have
  // a corresponding tool bar.
  std::multimap<QString, berry::IViewDescriptor::Pointer> GetViews()
  {
    std::multimap<QString, berry::IViewDescriptor::Pointer> result;

    const auto workbench = berry::PlatformUI::GetWorkbench();
    const auto viewRegistry = workbench->GetViewRegistry();
    const auto views = viewRegistry->GetViews();

    for (auto view : views)
    {
      QString category;

      if (auto categoryPath = view->GetCategoryPath(); !categoryPath.isEmpty())
        category = categoryPath.back();

      if (!category.isEmpty() && !category.contains('.') && !view->IsInternal())
        result.emplace(category, view);
    }

    return result;
  }

  // Get all toolbars of all (typically one) Workbench windows.
  std::vector<QToolBar*> GetToolBars()
  {
    std::vector<QToolBar*> result;

    const auto* workbench = berry::PlatformUI::GetWorkbench();
    auto workbenchWindows = workbench->GetWorkbenchWindows();

    for (auto workbenchWindow : workbenchWindows)
    {
      if (auto shell = workbenchWindow->GetShell(); shell.IsNotNull())
      {
        if (const auto* mainWindow = qobject_cast<QMainWindow*>(shell->GetControl()); mainWindow != nullptr)
        {
          for (auto child : mainWindow->children())
          {
            if (auto toolBar = qobject_cast<QToolBar*>(child); toolBar != nullptr)
              result.push_back(toolBar);
          }
        }
      }
    }

    return result;
  }

  // Find a toolbar by object name and apply visibility.
  bool ApplyVisibility(const std::vector<QToolBar*>& toolBars, const QString& name, bool isVisible)
  {
    auto it = std::find_if(toolBars.cbegin(), toolBars.cend(), [&name](const QToolBar* toolBar) {
      return toolBar->objectName() == name;
      });

    if (it != toolBars.cend())
    {
      (*it)->setVisible(isVisible);
      return true;
    }

    return false;
  }
}

QmitkToolBarsPreferencePage::QmitkToolBarsPreferencePage()
  : m_Ui(new Ui::QmitkToolBarsPreferencePage),
    m_Control(nullptr)
{
}

QmitkToolBarsPreferencePage::~QmitkToolBarsPreferencePage()
{
}

void QmitkToolBarsPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkToolBarsPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);

  m_Ui->setupUi(m_Control);

  const auto views = GetViews();

  for (auto category = views.cbegin(), end = views.cend(); category != end; category = views.upper_bound(category->first))
  {
    auto categoryItem = new QTreeWidgetItem;
    categoryItem->setText(0, category->first);
    categoryItem->setCheckState(0, Qt::Checked);

    const auto range = views.equal_range(category->first);

    for (auto view = range.first; view != range.second; ++view)
    {
      auto viewItem = new QTreeWidgetItem;
      viewItem->setText(0, view->second->GetLabel());

      categoryItem->addChild(viewItem);
    }

    m_Ui->treeWidget->addTopLevelItem(categoryItem);
  }

  this->Update();
}

QWidget* QmitkToolBarsPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkToolBarsPreferencePage::PerformOk()
{
  auto prefs = GetPreferences();
  const auto toolBars = GetToolBars();

  for (int i = 0, count = m_Ui->treeWidget->topLevelItemCount(); i < count; ++i)
  {
    const auto* item = m_Ui->treeWidget->topLevelItem(i);
    const auto category = item->text(0);
    const bool isVisible = item->checkState(0) == Qt::Checked;

    prefs->PutBool(category.toStdString(), isVisible);

    if (!ApplyVisibility(toolBars, category, isVisible))
      MITK_WARN << "Could not find tool bar for category \"" << category << "\" to set its visibility!";
  }

  return true;
}

void QmitkToolBarsPreferencePage::PerformCancel()
{
}

void QmitkToolBarsPreferencePage::Update()
{
  const auto prefs = GetPreferences();

  for (int i = 0, count = m_Ui->treeWidget->topLevelItemCount(); i < count; ++i)
  {
    auto item = m_Ui->treeWidget->topLevelItem(i);
    const auto category = item->text(0).toStdString();
    const bool isVisible = prefs->GetBool(category, true);

    item->setCheckState(0, isVisible ? Qt::Checked : Qt::Unchecked);
  }
}

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

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto prefService = mitk::CoreServices::GetPreferencesService();
    return prefService->GetSystemPreferences()->Node(QmitkApplicationConstants::TOOL_BARS_PREFERENCES);
  }

  // Find a toolbar by object name and apply preferences.
  bool ApplyPreferences(const QList<QToolBar*>& toolBars, const QString& name, bool isVisible, bool showCategory)
  {
    auto it = std::find_if(toolBars.cbegin(), toolBars.cend(), [&name](const QToolBar* toolBar) {
      return toolBar->objectName() == name;
    });

    if (it != toolBars.cend())
    {
      auto toolBar = *it;
      toolBar->setVisible(isVisible);

      for (auto action : toolBar->actions())
      {
        if (action->objectName() == "category")
        {
          action->setVisible(showCategory);
          break;
        }
      }

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

  const auto views = berry::PlatformUI::GetWorkbench()->GetViewRegistry()->GetViewsByCategory();

  for(const auto category : views.uniqueKeys())
  {
    auto categoryItem = new QTreeWidgetItem;
    categoryItem->setText(0, category);
    categoryItem->setCheckState(0, Qt::Checked);

    auto [viewIter, end] = views.equal_range(category);

    while (viewIter != end)
    {
      auto viewItem = new QTreeWidgetItem;
      viewItem->setText(0, (*viewIter)->GetLabel());
      viewItem->setIcon(0, (*viewIter)->GetImageDescriptor());

      categoryItem->addChild(viewItem);
      ++viewIter;
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
  bool showCategories = m_Ui->showCategoriesCheckBox->isChecked();

  prefs->PutBool(QmitkApplicationConstants::TOOL_BARS_SHOW_CATEGORIES, showCategories);

  const auto toolBars = berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows().first()->GetToolBars();

  for (int i = 0, count = m_Ui->treeWidget->topLevelItemCount(); i < count; ++i)
  {
    const auto* item = m_Ui->treeWidget->topLevelItem(i);
    const auto category = item->text(0);
    const bool isVisible = item->checkState(0) == Qt::Checked;

    prefs->PutBool(category.toStdString(), isVisible);

    if (!ApplyPreferences(toolBars, category, isVisible, showCategories))
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

  m_Ui->showCategoriesCheckBox->setChecked(prefs->GetBool(QmitkApplicationConstants::TOOL_BARS_SHOW_CATEGORIES, true));

  for (int i = 0, count = m_Ui->treeWidget->topLevelItemCount(); i < count; ++i)
  {
    auto item = m_Ui->treeWidget->topLevelItem(i);
    const auto category = item->text(0).toStdString();
    const bool isVisible = prefs->GetBool(category, true);

    item->setCheckState(0, isVisible ? Qt::Checked : Qt::Unchecked);
  }
}

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

#include "QmitkApplicationConstants.h"
#include "QmitkToolBarPresets.h"

#include <berryPlatformUI.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto prefService = mitk::CoreServices::GetPreferencesService();
    return prefService->GetSystemPreferences()->Node(QmitkApplicationConstants::TOOL_BARS_PREFERENCES);
  }
}

QmitkToolBarsPreferencePage::QmitkToolBarsPreferencePage()
  : m_Ui(std::make_unique<Ui::QmitkToolBarsPreferencePage>()),
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

  using CategoryLabel = QmitkCategoryToolBar::CategoryLabel;

  m_Ui->categoryLabelComboBox->addItem("Above the buttons", static_cast<int>(CategoryLabel::AboveButtons));
  m_Ui->categoryLabelComboBox->addItem("On hover", static_cast<int>(CategoryLabel::OnHover));
  m_Ui->categoryLabelComboBox->addItem("Hidden", static_cast<int>(CategoryLabel::Hidden));

  const auto views = berry::PlatformUI::GetWorkbench()->GetViewRegistry()->GetViewsByCategory();

  for(const auto& category : QmitkToolBarPresets::GetCategories())
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

  QmitkToolBarPresets::SetCategoryLabel(static_cast<QmitkCategoryToolBar::CategoryLabel>(m_Ui->categoryLabelComboBox->currentData().toInt()));

  for (int i = 0, count = m_Ui->treeWidget->topLevelItemCount(); i < count; ++i)
  {
    const auto* item = m_Ui->treeWidget->topLevelItem(i);
    const auto category = item->text(0).toStdString();
    const bool isVisible = item->checkState(0) == Qt::Checked;

    prefs->PutBool(category, isVisible);
  }

  QmitkToolBarPresets::ApplyToWorkbench();

  return true;
}

void QmitkToolBarsPreferencePage::PerformCancel()
{
}

void QmitkToolBarsPreferencePage::Update()
{
  const auto prefs = GetPreferences();

  m_Ui->categoryLabelComboBox->setCurrentIndex(m_Ui->categoryLabelComboBox->findData(static_cast<int>(QmitkToolBarPresets::GetCategoryLabel())));

  for (int i = 0, count = m_Ui->treeWidget->topLevelItemCount(); i < count; ++i)
  {
    auto item = m_Ui->treeWidget->topLevelItem(i);
    const auto category = item->text(0).toStdString();
    const bool isVisible = prefs->GetBool(category, true);

    item->setCheckState(0, isVisible ? Qt::Checked : Qt::Unchecked);
  }
}

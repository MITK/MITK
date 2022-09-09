/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkLabelsWidget.h"
#include <ui_QmitkLabelsWidgetControls.h>

// mitk
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkToolManagerProvider.h>

// Qmitk
#include <QmitkAbstractNodeSelectionWidget.h>
#include <QmitkStyleManager.h>
#include <QmitkNewSegmentationDialog.h>

#include "../QmitkSaveMultiLabelPresetAction.h"
#include "../QmitkLoadMultiLabelPresetAction.h"

// Qt
#include <QMessageBox>
#include <QShortcut>

QmitkLabelsWidget::QmitkLabelsWidget(QWidget *parent)
  : QWidget(parent)
  , m_Controls(new Ui::QmitkLabelsWidgetControls)
  , m_ToolManager(nullptr)
  , m_DefaultLabelNaming(true)
{
  m_Controls->setupUi(this);

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();

  m_Controls->savePresetButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-save.svg")));
  m_Controls->loadPresetButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-open.svg")));

  connect(m_Controls->newLabelButton, &QToolButton::clicked, this, &QmitkLabelsWidget::OnNewLabel);
  connect(m_Controls->lockExteriorButton, &QToolButton::toggled, this, &QmitkLabelsWidget::OnLockExterior);
  connect(m_Controls->savePresetButton, &QToolButton::clicked, this, &QmitkLabelsWidget::OnSavePreset);
  connect(m_Controls->loadPresetButton, &QToolButton::clicked, this, &QmitkLabelsWidget::OnLoadPreset);
  connect(m_Controls->showLabelTableButton, &QToolButton::toggled, this, &QmitkLabelsWidget::ShowLabelTable);

  auto* newLabelShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key::Key_L, Qt::CTRL | Qt::Key::Key_N), this);
  connect(newLabelShortcut, &QShortcut::activated, this, &QmitkLabelsWidget::OnNewLabelShortcutActivated);

  this->UpdateGUI();
}

QmitkLabelsWidget::~QmitkLabelsWidget()
{
  delete m_Controls;
}

void QmitkLabelsWidget::UpdateGUI()
{
  m_Controls->newLabelButton->setEnabled(false);
  m_Controls->lockExteriorButton->setEnabled(false);
  m_Controls->lockExteriorButton->setChecked(false);
  m_Controls->savePresetButton->setEnabled(false);
  m_Controls->loadPresetButton->setEnabled(false);
  m_Controls->showLabelTableButton->setEnabled(false);
  m_Controls->showLabelTableButton->setChecked(false);

  mitk::LabelSetImage* workingImage = this->GetWorkingImage();
  if (nullptr == workingImage)
  {
    return;
  }

  int activeLayer = workingImage->GetActiveLayer();
  m_Controls->lockExteriorButton->setEnabled(true);
  m_Controls->lockExteriorButton->setChecked(workingImage->GetLabel(0, activeLayer)->GetLocked());
  m_Controls->showLabelTableButton->setEnabled(true);
  m_Controls->showLabelTableButton->setChecked(true);
  m_Controls->newLabelButton->setEnabled(true);
  m_Controls->savePresetButton->setEnabled(true);
  m_Controls->loadPresetButton->setEnabled(true);
}

void QmitkLabelsWidget::SetDefaultLabelNaming(bool defaultLabelNaming)
{
  m_DefaultLabelNaming = defaultLabelNaming;
}

mitk::LabelSetImage* QmitkLabelsWidget::GetWorkingImage()
{
  mitk::DataNode* workingNode = this->GetWorkingNode();
  if (nullptr == workingNode)
  {
    return nullptr;
  }

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  return workingImage;
}

mitk::DataNode* QmitkLabelsWidget::GetWorkingNode()
{
  mitk::DataNode* referenceNode = m_ToolManager->GetWorkingData(0);
  return referenceNode;
}

void QmitkLabelsWidget::OnNewLabel()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = this->GetWorkingNode();
  if (nullptr == workingNode)
  {
    return;
  }

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (nullptr == workingImage)
  {
    return;
  }

  mitk::Label::Pointer newLabel = mitk::LabelSetImageHelper::CreateNewLabel(workingImage);

  if (!m_DefaultLabelNaming)
  {
    QmitkNewSegmentationDialog dialog(this, workingImage);
    dialog.SetName(QString::fromStdString(newLabel->GetName()));
    dialog.SetColor(newLabel->GetColor());

    if (QDialog::Rejected == dialog.exec())
      return;

    auto name = dialog.GetName();

    if (!name.isEmpty())
      newLabel->SetName(name.toStdString());

    newLabel->SetColor(dialog.GetColor());
  }

  workingImage->GetActiveLabelSet()->AddLabel(newLabel);

  this->UpdateGUI();
  emit LabelsChanged();
}

void QmitkLabelsWidget::OnNewLabelShortcutActivated()
{
  m_Controls->newLabelButton->click();
}

void QmitkLabelsWidget::OnLockExterior(bool checked)
{
  auto workingImage = this->GetWorkingImage();
  if (nullptr == workingImage)
  {
    return;
  }

  workingImage->GetLabel(0)->SetLocked(checked);
}

void QmitkLabelsWidget::OnSavePreset()
{
  auto workingNode = this->GetWorkingNode();
  QmitkAbstractNodeSelectionWidget::NodeList nodes;
  nodes.append(workingNode);

  QmitkSaveMultiLabelPresetAction action;
  action.Run(nodes);
}

void QmitkLabelsWidget::OnLoadPreset()
{
  auto workingNode = this->GetWorkingNode();
  QmitkAbstractNodeSelectionWidget::NodeList nodes;
  nodes.append(workingNode);

  QmitkLoadMultiLabelPresetAction action;
  action.Run(nodes);
}

void QmitkLabelsWidget::WaitCursorOn()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void QmitkLabelsWidget::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkLabelsWidget::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}


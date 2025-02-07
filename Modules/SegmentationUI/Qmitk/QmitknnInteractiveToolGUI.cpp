/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitknnInteractiveToolGUI.h"
#include <ui_QmitknnInteractiveToolGUI.h>

#include <QmitkStyleManager.h>

#include <QBoxLayout>
#include <QButtonGroup>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitknnInteractiveToolGUI, "")

using Self = QmitknnInteractiveToolGUI;

QmitknnInteractiveToolGUI::QmitknnInteractiveToolGUI()
  : QmitkSegWithPreviewToolGUIBase(false),
    m_Ui(new Ui::QmitknnInteractiveToolGUI),
    m_PromptTypeButtonGroup(new QButtonGroup(this)),
    m_PromptType(mitk::nnInteractiveTool::PromptType::Positive)
{
}

QmitknnInteractiveToolGUI::~QmitknnInteractiveToolGUI()
{
}

void QmitknnInteractiveToolGUI::InitializeUI(QBoxLayout* mainLayout)
{
  auto wrapperWidget = new QWidget(this);
  mainLayout->addWidget(wrapperWidget);
  m_Ui->setupUi(wrapperWidget);

  this->ThemeIcons();

  connect(m_Ui->initializeButton, &QPushButton::toggled, this, &Self::OnInitializeButtonToggled);

  this->InitializePromptType();
  this->InitializeToolButtons();

  Superclass::InitializeUI(mainLayout);
}

void QmitknnInteractiveToolGUI::ThemeIcons()
{
  m_Ui->resetButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/nnInteractive/reset")));
  m_Ui->positiveButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/nnInteractive/positive")));
  m_Ui->negativeButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/nnInteractive/negative")));
  m_Ui->pointButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/nnInteractive/point")));
  m_Ui->boxButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/nnInteractive/box")));
  m_Ui->scribbleButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/nnInteractive/scribble")));
  m_Ui->lassoButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/nnInteractive/lasso")));
}

void QmitknnInteractiveToolGUI::InitializePromptType()
{
  m_PromptType = m_Ui->positiveButton->isChecked()
    ? mitk::nnInteractiveTool::PromptType::Positive
    : mitk::nnInteractiveTool::PromptType::Negative;

  m_PromptTypeButtonGroup->addButton(m_Ui->positiveButton);
  m_PromptTypeButtonGroup->setId(m_Ui->positiveButton, mitk::nnInteractiveTool::PromptType::Positive);

  m_PromptTypeButtonGroup->addButton(m_Ui->negativeButton);
  m_PromptTypeButtonGroup->setId(m_Ui->negativeButton, mitk::nnInteractiveTool::PromptType::Negative);

  auto idClicked = [this](int id)
  {
    if (id != m_PromptType)
    {
      m_PromptType = static_cast<mitk::nnInteractiveTool::PromptType>(id);
      this->OnPromptTypeChanged();
    }
  };

  connect(m_PromptTypeButtonGroup, &QButtonGroup::idClicked, idClicked);
}

void QmitknnInteractiveToolGUI::InitializeToolButtons()
{
  connect(m_Ui->pointButton, &QPushButton::toggled, this, &Self::OnPointToolToggled);
  connect(m_Ui->boxButton, &QPushButton::toggled, this, &Self::OnBoxToolToggled);
  connect(m_Ui->scribbleButton, &QPushButton::toggled, this, &Self::OnScribbleToolToggled);
  connect(m_Ui->lassoButton, &QPushButton::toggled, this, &Self::OnLassoToolToggled);
}

void QmitknnInteractiveToolGUI::OnInitializeButtonToggled(bool checked)
{
  m_Ui->initializeButton->setEnabled(!checked);

  m_Ui->resetButton->setEnabled(checked);
  m_Ui->promptTypeGroupBox->setEnabled(checked);
  m_Ui->interactionToolsGroupBox->setEnabled(checked);
}

void QmitknnInteractiveToolGUI::OnPromptTypeChanged()
{
}

void QmitknnInteractiveToolGUI::OnPointToolToggled(bool checked)
{
  if (checked)
  {
    this->SwitchOffOtherTools(m_Ui->pointButton);
  }
  else
  {
  }
}

void QmitknnInteractiveToolGUI::OnBoxToolToggled(bool checked)
{
  if (checked)
  {
    this->SwitchOffOtherTools(m_Ui->boxButton);
  }
  else
  {
  }
}

void QmitknnInteractiveToolGUI::OnScribbleToolToggled(bool checked)
{
  if (checked)
  {
    this->SwitchOffOtherTools(m_Ui->scribbleButton);
  }
  else
  {
  }
}

void QmitknnInteractiveToolGUI::OnLassoToolToggled(bool checked)
{
  if (checked)
  {
    this->SwitchOffOtherTools(m_Ui->lassoButton);
  }
  else
  {
  }
}

void QmitknnInteractiveToolGUI::SwitchOffOtherTools(QPushButton* toolButton)
{
  for (auto otherToolButton : m_Ui->interactionToolsGroupBox->findChildren<QPushButton*>())
  {
    if (otherToolButton != toolButton)
      otherToolButton->setChecked(false);
  }
}

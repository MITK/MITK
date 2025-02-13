/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitknnInteractiveToolGUI.h"
#include <ui_QmitknnInteractiveToolGUI.h>

#include <mitkPointSetDataInteractor.h>

#include <QmitkStyleManager.h>

#include <QBoxLayout>
#include <QButtonGroup>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitknnInteractiveToolGUI, "")

QmitknnInteractiveToolGUI::QmitknnInteractiveToolGUI()
  : QmitkSegWithPreviewToolGUIBase(false),
    m_Ui(new Ui::QmitknnInteractiveToolGUI),
    m_PromptTypeButtonGroup(new QButtonGroup(this)),
    m_PromptType(PromptType::Positive)
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
    ? PromptType::Positive
    : PromptType::Negative;

  m_PromptTypeButtonGroup->addButton(m_Ui->positiveButton);
  m_PromptTypeButtonGroup->setId(m_Ui->positiveButton, PromptType::Positive);

  m_PromptTypeButtonGroup->addButton(m_Ui->negativeButton);
  m_PromptTypeButtonGroup->setId(m_Ui->negativeButton, PromptType::Negative);

  auto idClicked = [this](int id)
  {
    if (id != m_PromptType)
    {
      m_PromptType = static_cast<PromptType>(id);
      this->OnPromptTypeChanged();
    }
  };

  connect(m_PromptTypeButtonGroup, &QButtonGroup::idClicked, idClicked);
}

void QmitknnInteractiveToolGUI::InitializeToolButtons()
{
  m_ToolButtons[Tool::Point] = m_Ui->pointButton;
  m_ToolButtons[Tool::Box] = m_Ui->boxButton;
  m_ToolButtons[Tool::Scribble] = m_Ui->scribbleButton;
  m_ToolButtons[Tool::Lasso] = m_Ui->lassoButton;

  for (const auto& [tool, button] : m_ToolButtons)
    connect(button, &QPushButton::toggled, [this, tool](bool checked) { this->OnToolToggled(tool, checked); });
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
  for (auto tool : this->GetTool()->GetTools())
  {
    if (m_ToolButtons[tool]->isChecked())
    {
      this->GetTool()->EnableInteraction(tool, m_PromptType);
      break;
    }
  }
}

void QmitknnInteractiveToolGUI::OnToolToggled(Tool tool, bool checked)
{
  if (checked)
  {
    this->UncheckOtherToolButtons(m_ToolButtons[tool]);
    this->GetTool()->EnableInteraction(tool, m_PromptType);
  }
  else
  {
    this->GetTool()->DisableInteraction();
  }
}

void QmitknnInteractiveToolGUI::UncheckOtherToolButtons(QPushButton* toolButton)
{
  for (const auto& [tool, button] : m_ToolButtons)
  {
    if (button != toolButton)
      button->setChecked(false);
  }
}

mitk::nnInteractiveTool* QmitknnInteractiveToolGUI::GetTool()
{
  return this->GetConnectedToolAs<mitk::nnInteractiveTool>();
}

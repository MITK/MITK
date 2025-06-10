/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkEditableContourToolGUIBase.h>
#include <QmitkStyleManager.h>

#include <ui_QmitkEditableContourToolGUIControls.h>

#include <QButtonGroup>

#include <mitkEditableContourTool.h>

QmitkEditableContourToolGUIBase::QmitkEditableContourToolGUIBase()
  : QmitkToolGUI(),
    m_Controls(new Ui::QmitkEditableContourToolGUIControls)
{
  m_Controls->setupUi(this);

  m_Controls->m_ConfirmButton->hide();
  m_Controls->m_ClearButton->hide();
  m_Controls->m_Information->hide();

  m_Controls->m_AddBtn->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/lasso_mode_add.svg")));
  m_Controls->m_SubtractBtn->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/lasso_mode_subtract.svg")));

  connect(m_Controls->m_AddBtn, &QPushButton::clicked, this, [=](bool)
    {
      m_Controls->m_AddBtn->setChecked(true);
      m_Controls->m_SubtractBtn->setChecked(false);
      this->OnModeToggled(Mode::Add);
    });
  connect(m_Controls->m_SubtractBtn, &QPushButton::clicked, this, [=](bool)
    {
      m_Controls->m_SubtractBtn->setChecked(true);
      m_Controls->m_AddBtn->setChecked(false);
      this->OnModeToggled(Mode::Subtract);
    });

  connect(this, &Self::NewToolAssociated, this, &Self::OnNewToolAssociated);
  connect(m_Controls->m_AutoCheck, &QCheckBox::toggled, this, &Self::OnAutoConfirm);
  connect(m_Controls->m_ConfirmButton, &QPushButton::clicked, this, &Self::OnConfirmSegmentation);
  connect(m_Controls->m_ClearButton, &QPushButton::clicked, this, &Self::OnClearContour);
  connect(m_Controls->m_InformationCheckBox, &QCheckBox::toggled, this, &Self::OnShowInformation);
}

QmitkEditableContourToolGUIBase::~QmitkEditableContourToolGUIBase()
{
}

void QmitkEditableContourToolGUIBase::OnNewToolAssociated(mitk::Tool* tool)
{
  m_NewTool = dynamic_cast<mitk::EditableContourTool*>(tool);

  if (m_NewTool.IsNull())
    mitkThrow() << "Tool is in an invalid state. QmitkEditableContourToolGUIBase needs tools based on EditableContourTool.";

  const auto autoConfirm = m_NewTool->GetAutoConfirm();
  m_Controls->m_AutoCheck->setChecked(autoConfirm);

  const auto mode = m_NewTool->GetAddMode()
    ? Mode::Add
    : Mode::Subtract;

  if (m_NewTool->GetAddMode())
    m_Controls->m_AddBtn->click();
  else
    m_Controls->m_SubtractBtn->click();

  this->OnAutoConfirm(autoConfirm);
  this->OnModeToggled(mode);
}

void QmitkEditableContourToolGUIBase::OnAutoConfirm(bool on)
{
  m_Controls->m_ConfirmButton->setVisible(!on);
  m_Controls->m_ClearButton->setVisible(!on);

  if (m_NewTool.IsNotNull())
  {
    if (on && m_NewTool->IsEditingContour())
      this->OnConfirmSegmentation();

    m_NewTool->SetAutoConfirm(on);
    m_NewTool->SetAddMode(m_Controls->m_AddBtn->isChecked());
  }
}

void QmitkEditableContourToolGUIBase::OnModeToggled(Mode mode)
{
  if (m_NewTool.IsNotNull())
    m_NewTool->SetAddMode(Mode::Add == mode);
}

void QmitkEditableContourToolGUIBase::OnConfirmSegmentation()
{
  if (m_NewTool.IsNotNull())
    m_NewTool->ConfirmSegmentation();
}

void QmitkEditableContourToolGUIBase::OnClearContour()
{
  if (m_NewTool.IsNotNull())
    m_NewTool->ClearContour();
}

void QmitkEditableContourToolGUIBase::OnShowInformation(bool on)
{
  m_Controls->m_Information->setVisible(on);
}

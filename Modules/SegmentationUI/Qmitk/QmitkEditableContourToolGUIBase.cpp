/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkEditableContourToolGUIBase.h>
#include <ui_QmitkEditableContourToolGUIControls.h>

#include <mitkEditableContourTool.h>

QmitkEditableContourToolGUIBase::QmitkEditableContourToolGUIBase()
  : QmitkToolGUI(),
    m_Controls(new Ui::QmitkEditableContourToolGUIControls)
{
  m_Controls->setupUi(this);

  m_Controls->m_Information->hide();
  m_Controls->m_AutoCheck->setChecked(true);
  m_Controls->m_ConfirmButton->hide();
  m_Controls->m_AddMode->setChecked(true);
  m_Controls->m_SubtractMode->hide();
  m_Controls->m_AddMode->hide();
  m_Controls->m_ClearButton->hide();

  connect(m_Controls->m_ConfirmButton, &QPushButton::clicked, this, &Self::OnConfirmSegmentation);
  connect(m_Controls->m_ClearButton, &QPushButton::clicked, this, &Self::OnClearContour);
  connect(this, &Self::NewToolAssociated, this, &Self::OnNewToolAssociated);
  connect(m_Controls->m_InformationCheckBox, &QCheckBox::toggled, this, &Self::OnShowInformation);
  connect(m_Controls->m_AutoCheck, &QCheckBox::toggled, this, &Self::OnAutoConfirm);
  connect(m_Controls->m_AddMode, &QRadioButton::toggled, this, &Self::OnAddModeToogled);
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

  const auto addMode = m_NewTool->GetAddMode();
  m_Controls->m_AddMode->setChecked(addMode);

  this->OnAutoConfirm(autoConfirm);
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

void QmitkEditableContourToolGUIBase::OnAutoConfirm(bool on)
{
  m_Controls->m_ConfirmButton->setVisible(!on);
  m_Controls->m_ClearButton->setVisible(!on);
  m_Controls->m_AddMode->setVisible(!on);

  if (on)
    m_Controls->m_AddMode->setChecked(true);

  m_Controls->m_SubtractMode->setVisible(!on);

  if (m_NewTool.IsNotNull())
  {
    if (on && m_NewTool->IsEditingContour())
      this->OnConfirmSegmentation();

    m_NewTool->SetAutoConfirm(on);
  }
}

void QmitkEditableContourToolGUIBase::OnAddModeToogled(bool on)
{
  if (m_NewTool.IsNotNull())
    m_NewTool->SetAddMode(on);
}

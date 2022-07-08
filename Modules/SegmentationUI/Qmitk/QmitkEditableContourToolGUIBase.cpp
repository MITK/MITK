/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkEditableContourToolGUIBase.h"

QmitkEditableContourToolGUIBase::QmitkEditableContourToolGUIBase() : QmitkToolGUI()
{
  m_Controls.setupUi(this);
  m_Controls.m_Information->hide();
  m_Controls.m_AutoCheck->setChecked(true);
  m_Controls.m_ConfirmButton->hide();
  m_Controls.m_AddMode->setChecked(true);
  m_Controls.m_SubstractMode->hide();
  m_Controls.m_AddMode->hide();

  m_Controls.m_ClearButton->hide();

  connect(m_Controls.m_ConfirmButton, SIGNAL(clicked()), this, SLOT(OnConfirmSegmentation()));
  connect(m_Controls.m_ClearButton, SIGNAL(clicked()), this, SLOT(OnClearSegmentation()));
  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
  connect(m_Controls.m_InformationCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)));
  connect(m_Controls.m_AutoCheck, SIGNAL(toggled(bool)), this, SLOT(OnAutoConfirm(bool)));
}

QmitkEditableContourToolGUIBase::~QmitkEditableContourToolGUIBase()
{
}

void QmitkEditableContourToolGUIBase::OnNewToolAssociated(mitk::Tool *tool)
{
  m_NewTool = dynamic_cast<mitk::EditableContourTool *>(tool);
  m_Controls.m_AutoCheck->setChecked(m_NewTool->GetAutoConfirm());
}

void QmitkEditableContourToolGUIBase::OnConfirmSegmentation()
{
  if (m_NewTool.IsNotNull())
  {
    m_NewTool->SetAddMode(m_Controls.m_AddMode->isChecked());
    m_NewTool->ConfirmSegmentation();
  }
}

void QmitkEditableContourToolGUIBase::OnClearSegmentation()
{
  if (m_NewTool.IsNotNull())
    m_NewTool->ClearSegmentation();
}

void QmitkEditableContourToolGUIBase::OnShowInformation(bool on)
{
  m_Controls.m_Information->setVisible(on);
}

void QmitkEditableContourToolGUIBase::OnAutoConfirm(bool on)
{
  m_Controls.m_ConfirmButton->setVisible(!on);
  m_Controls.m_ClearButton->setVisible(!on);
  m_Controls.m_AddMode->setVisible(!on);
  m_Controls.m_SubstractMode->setVisible(!on);
  m_NewTool->SetAutoConfirm(on);
}


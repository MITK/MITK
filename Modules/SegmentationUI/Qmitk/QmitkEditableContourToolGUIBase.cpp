/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkEditableContourToolGUIBase.h>
#include <ui_QmitkEditableContourToolGUIControls.h>

#include <QButtonGroup>

#include <mitkEditableContourTool.h>

QmitkEditableContourToolGUIBase::QmitkEditableContourToolGUIBase()
  : QmitkToolGUI(),
    m_Controls(new Ui::QmitkEditableContourToolGUIControls),
    m_ModeButtonGroup(new QButtonGroup(this))
{
  m_Controls->setupUi(this);

  m_Controls->m_ConfirmButton->hide();
  m_Controls->m_AddMode->hide();
  m_Controls->m_SubtractMode->hide();
  m_Controls->m_ClearButton->hide();
  m_Controls->m_Information->hide();

  m_ModeButtonGroup->addButton(m_Controls->m_AddMode, static_cast<int>(Mode::Add));
  m_ModeButtonGroup->addButton(m_Controls->m_SubtractMode, static_cast<int>(Mode::Subtract));

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
  auto onModeToggled = [this](QAbstractButton* button) {
    Mode mode = button == m_Controls->m_AddMode
      ? Mode::Add
      : Mode::Subtract;

    this->OnModeToggled(mode);
  };

  connect(m_ModeButtonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), onModeToggled);
#else
  connect(m_ModeButtonGroup, &QButtonGroup::idClicked, [this](int id) { this->OnModeToggled(static_cast<Mode>(id)); });
#endif

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

  m_ModeButtonGroup->button(static_cast<int>(mode))->setChecked(true);

  this->OnAutoConfirm(autoConfirm);
  this->OnModeToggled(mode);
}

void QmitkEditableContourToolGUIBase::OnAutoConfirm(bool on)
{
  m_Controls->m_ConfirmButton->setVisible(!on);
  m_Controls->m_ClearButton->setVisible(!on);
  m_Controls->m_AddMode->setVisible(!on);
  m_Controls->m_SubtractMode->setVisible(!on);

  if (on)
    m_Controls->m_AddMode->setChecked(true);

  if (m_NewTool.IsNotNull())
  {
    if (on && m_NewTool->IsEditingContour())
      this->OnConfirmSegmentation();

    m_NewTool->SetAutoConfirm(on);
    m_NewTool->SetAddMode(m_Controls->m_AddMode->isChecked());
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

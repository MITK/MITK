/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPickingToolGUI.h"

#include <mitkPickingTool.h>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QBoxLayout>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkPickingToolGUI, "")

QmitkPickingToolGUI::QmitkPickingToolGUI() : QmitkSegWithPreviewToolGUIBase(false)
{
  auto enablePickingDelegate = [this](bool enabled)
  {
    bool result = false;
    auto tool = this->GetConnectedToolAs<mitk::PickingTool>();
    if (nullptr != tool)
    {
      result = enabled && tool->HasPicks();
    }

    return result;
  };

  m_EnableConfirmSegBtnFnc = enablePickingDelegate;
}

QmitkPickingToolGUI::~QmitkPickingToolGUI()
{
}

void QmitkPickingToolGUI::OnResetPicksClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::PickingTool>();
  if (nullptr != tool)
  {
    tool->ClearPicks();
  }
}

void QmitkPickingToolGUI::OnRadioPickClicked(bool checked)
{
  if (checked)
  {
    this->SetMergeStyle(mitk::MultiLabelSegmentation::MergeStyle::Replace);
    this->SetOverwriteStyle(mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
  }
  else
  {
    this->SetMergeStyle(mitk::MultiLabelSegmentation::MergeStyle::Merge);
    this->SetOverwriteStyle(mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
  }
}

void QmitkPickingToolGUI::InitializeUI(QBoxLayout* mainLayout)
{
  auto radioPick = new QRadioButton("Picking mode", this);
  radioPick->setToolTip("Pick certain parts of the label and dismiss the rest of the label content");
  radioPick->setChecked(true);
  connect(radioPick, &QAbstractButton::toggled, this, &QmitkPickingToolGUI::OnRadioPickClicked);
  mainLayout->addWidget(radioPick);
  m_RadioPick = radioPick;
  this->OnRadioPickClicked(true);

  auto radioRelabel = new QRadioButton("Relabel mode", this);
  radioRelabel->setToolTip("Relabel certain parts of the segmentation as active label.");
  radioRelabel->setChecked(false);
  mainLayout->addWidget(radioRelabel);
  m_RadioRelabel = radioRelabel;

  QLabel* label = new QLabel("Press SHIFT and click to pick region(s).\nPress DEL to remove last pick.", this);
  mainLayout->addWidget(label);

  auto clearButton = new QPushButton("Reset picks",this);
  connect(clearButton, &QPushButton::clicked, this, &QmitkPickingToolGUI::OnResetPicksClicked);
  mainLayout->addWidget(clearButton);
  m_ClearPicksBtn = clearButton;

  Superclass::InitializeUI(mainLayout);
}

void QmitkPickingToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);

  if (nullptr != m_ClearPicksBtn)
  {
    m_ClearPicksBtn->setEnabled(enabled);
  }
  if (nullptr != m_RadioPick)
  {
    m_RadioPick->setEnabled(enabled);
  }
  if (nullptr != m_RadioRelabel)
  {
    m_RadioRelabel->setEnabled(enabled);
  }
}


/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPickingToolGUI.h"

#include <mitkPickingTool.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qboxlayout.h>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkPickingToolGUI, "")

QmitkPickingToolGUI::QmitkPickingToolGUI() : QmitkAutoSegmentationToolGUIBase(false)
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

void QmitkPickingToolGUI::InitializeUI(QBoxLayout* mainLayout)
{
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
}


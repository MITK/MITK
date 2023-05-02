/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentAnythingToolGUI.h"

#include <mitkSegmentAnythingTool.h>
#include <QLabel> // Mirgrate all to QML file
#include <QPushButton>
#include <QRadioButton>
#include <QBoxLayout>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkSegmentAnythingToolGUI, "")

QmitkSegmentAnythingToolGUI::QmitkSegmentAnythingToolGUI() : QmitkSegWithPreviewToolGUIBase(true)
{
  auto enablePickingDelegate = [this](bool enabled)
  {
    bool result = false;
    auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
    if (nullptr != tool)
    {
      result = enabled && tool->HasPicks();
    }

    return result;
  };

  m_EnableConfirmSegBtnFnc = enablePickingDelegate;
}

void QmitkSegmentAnythingToolGUI::OnResetPicksClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
  if (nullptr != tool)
  {
    tool->ClearPicks();
  }
}

void QmitkSegmentAnythingToolGUI::InitializeUI(QBoxLayout* mainLayout)
{
  QLabel* welcomeLabel = new QLabel("Segment Anything Model Tool. [Experimental]", this);
  mainLayout->addWidget(welcomeLabel);

  auto radioPick = new QRadioButton("Interactive", this);
  radioPick->setToolTip("Click on certain parts of the image to segment.");
  radioPick->setChecked(true);
  mainLayout->addWidget(radioPick);
  m_RadioPick = radioPick;

  auto radioAutoMode = new QRadioButton("Auto", this);
  radioAutoMode->setToolTip("Automatic segmentation without clicks.");
  radioAutoMode->setChecked(false);
  mainLayout->addWidget(radioAutoMode);
  m_RadioRelabel = radioAutoMode;

  QLabel* clickLabel = new QLabel("Press SHIFT and click to pick region(s).\nPress DEL to remove last pick.", this);
  mainLayout->addWidget(clickLabel);

  auto clearButton = new QPushButton("Reset picks",this);
  connect(clearButton, &QPushButton::clicked, this, &QmitkSegmentAnythingToolGUI::OnResetPicksClicked);
  mainLayout->addWidget(clearButton);
  m_ClearPicksBtn = clearButton;

  auto activate = new QPushButton("Activate", this);
  // connect(clearButton, &QPushButton::clicked, this, &QmitkSegmentAnythingToolGUI::OnResetPicksClicked);
  mainLayout->addWidget(activate);

  Superclass::InitializeUI(mainLayout);
}

void QmitkSegmentAnythingToolGUI::EnableWidgets(bool enabled)
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


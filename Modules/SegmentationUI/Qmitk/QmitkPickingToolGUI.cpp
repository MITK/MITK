/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPickingToolGUI.h"

#include "QmitkNewSegmentationDialog.h"

#include "mitkBaseRenderer.h"
#include "mitkStepper.h"
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qslider.h>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkPickingToolGUI, "")

QmitkPickingToolGUI::QmitkPickingToolGUI() : QmitkToolGUI()
{
  m_Controls.setupUi(this);

  connect(m_Controls.m_ConfirmButton, SIGNAL(clicked()), this, SLOT(OnConfirmSegmentation()));

  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
}

QmitkPickingToolGUI::~QmitkPickingToolGUI()
{
}

void QmitkPickingToolGUI::OnNewToolAssociated(mitk::Tool *tool)
{
  m_PickingTool = dynamic_cast<mitk::PickingTool *>(tool);
}

void QmitkPickingToolGUI::OnConfirmSegmentation()
{
  if (m_PickingTool.IsNotNull())
  {
    m_PickingTool->ConfirmSegmentation();
  }
}

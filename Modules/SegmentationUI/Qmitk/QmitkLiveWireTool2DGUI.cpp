/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkLiveWireTool2DGUI.h"

#include "QmitkNewSegmentationDialog.h"

#include "mitkBaseRenderer.h"
#include "mitkStepper.h"
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qslider.h>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkLiveWireTool2DGUI, "")

QmitkLiveWireTool2DGUI::QmitkLiveWireTool2DGUI() : QmitkToolGUI()
{
  m_Controls.setupUi(this);
  m_Controls.m_Information->hide();

  connect(m_Controls.m_ConfirmButton, SIGNAL(clicked()), this, SLOT(OnConfirmSegmentation()));
  connect(m_Controls.m_ClearButton, SIGNAL(clicked()), this, SLOT(OnClearSegmentation()));
  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
  connect(m_Controls.m_InformationCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)));
}

QmitkLiveWireTool2DGUI::~QmitkLiveWireTool2DGUI()
{
}

void QmitkLiveWireTool2DGUI::OnNewToolAssociated(mitk::Tool *tool)
{
  m_LiveWireTool = dynamic_cast<mitk::LiveWireTool2D *>(tool);
}

void QmitkLiveWireTool2DGUI::OnConfirmSegmentation()
{
  if (m_LiveWireTool.IsNotNull())
    m_LiveWireTool->ConfirmSegmentation();
}

void QmitkLiveWireTool2DGUI::OnClearSegmentation()
{
  if (m_LiveWireTool.IsNotNull())
    m_LiveWireTool->ClearSegmentation();
}

void QmitkLiveWireTool2DGUI::OnShowInformation(bool on)
{
  m_Controls.m_Information->setVisible(on);
}

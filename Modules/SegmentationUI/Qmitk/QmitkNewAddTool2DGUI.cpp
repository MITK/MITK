/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNewAddTool2DGUI.h"

#include "mitkBaseRenderer.h"
#include "mitkStepper.h"
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qslider.h>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkNewAddTool2DGUI, "")

QmitkNewAddTool2DGUI::QmitkNewAddTool2DGUI() : QmitkToolGUI()
{
  m_Controls.setupUi(this);
  m_Controls.m_Information->hide();

  connect(m_Controls.m_ConfirmButton, SIGNAL(clicked()), this, SLOT(OnConfirmSegmentation()));
  connect(m_Controls.m_ClearButton, SIGNAL(clicked()), this, SLOT(OnClearSegmentation()));
  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
  connect(m_Controls.m_InformationCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)));
}

QmitkNewAddTool2DGUI::~QmitkNewAddTool2DGUI()
{
}

void QmitkNewAddTool2DGUI::OnNewToolAssociated(mitk::Tool *tool)
{
  m_NewAddTool = dynamic_cast<mitk::NewAddTool2D *>(tool);
}

void QmitkNewAddTool2DGUI::OnConfirmSegmentation()
{
  if (m_NewAddTool.IsNotNull())
    m_NewAddTool->ConfirmSegmentation();
}

void QmitkNewAddTool2DGUI::OnClearSegmentation()
{
  if (m_NewAddTool.IsNotNull())
    m_NewAddTool->ClearSegmentation();
}

void QmitkNewAddTool2DGUI::OnShowInformation(bool on)
{
  m_Controls.m_Information->setVisible(on);
}

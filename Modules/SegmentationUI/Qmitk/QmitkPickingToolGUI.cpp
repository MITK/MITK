/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkPickingToolGUI.h"

#include "QmitkNewSegmentationDialog.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>
#include "mitkStepper.h"
#include "mitkBaseRenderer.h"


MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkPickingToolGUI, "")

QmitkPickingToolGUI::QmitkPickingToolGUI()
:QmitkToolGUI()
{
  m_Controls.setupUi(this);

  connect( m_Controls.m_ConfirmButton, SIGNAL(clicked()), this, SLOT(OnConfirmSegmentation()) );

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );

}

QmitkPickingToolGUI::~QmitkPickingToolGUI()
{
}

void QmitkPickingToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  m_PickingTool = dynamic_cast<mitk::PickingTool*>( tool );
}


void QmitkPickingToolGUI::OnConfirmSegmentation()
{
  if (m_PickingTool.IsNotNull())
  {
    m_PickingTool->ConfirmSegmentation();
  }
}


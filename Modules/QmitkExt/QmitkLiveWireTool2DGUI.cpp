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

#include "QmitkLiveWireTool2DGUI.h"

#include "QmitkNewSegmentationDialog.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>
#include "mitkStepper.h"
#include "mitkBaseRenderer.h"


MITK_TOOL_GUI_MACRO(QmitkExt_EXPORT, QmitkLiveWireTool2DGUI, "")

QmitkLiveWireTool2DGUI::QmitkLiveWireTool2DGUI()
:QmitkToolGUI()
{
  this->setContentsMargins( 0, 0, 0, 0 );

  // create the visible widgets
  QGridLayout *layout = new QGridLayout(this);

  m_ConfirmButton = new QPushButton("Confirm Segmentation");
  layout->addWidget(m_ConfirmButton, 0,0);
  connect( m_ConfirmButton, SIGNAL(clicked()), this, SLOT(OnConfirmSegmentation()) );

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkLiveWireTool2DGUI::~QmitkLiveWireTool2DGUI()
{
}

void QmitkLiveWireTool2DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  m_LiveWireTool = dynamic_cast<mitk::LiveWireTool2D*>( tool );
}


void QmitkLiveWireTool2DGUI::OnConfirmSegmentation()
{
  if (m_LiveWireTool.IsNotNull())
  {
    m_LiveWireTool->ConfirmSegmentation();
  }
}

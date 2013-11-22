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

#include "QmitkCloseTool3DGUI.h"

#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkCloseTool3DGUI, "")

QmitkCloseTool3DGUI::QmitkCloseTool3DGUI() : QmitkToolGUI()
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbDifference, SIGNAL(clicked()), this, SLOT(OnCalculateDifference()) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkCloseTool3DGUI::~QmitkCloseTool3DGUI()
{
  if (m_CloseTool3D.IsNotNull())
  {
    m_CloseTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkCloseTool3DGUI, bool>( this, &QmitkCloseTool3DGUI::BusyStateChanged );
  }
}

void QmitkCloseTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_CloseTool3D.IsNotNull())
  {
    m_CloseTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkCloseTool3DGUI, bool>( this, &QmitkCloseTool3DGUI::BusyStateChanged );
  }

  m_CloseTool3D = dynamic_cast<mitk::CloseTool3D*>( tool );

  if (m_CloseTool3D.IsNotNull())
  {
    m_CloseTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkCloseTool3DGUI, bool>( this, &QmitkCloseTool3DGUI::BusyStateChanged );
  }
}

void QmitkCloseTool3DGUI::OnRun()
{
  if (m_CloseTool3D.IsNotNull())
  {
    m_CloseTool3D->Run();
  }
}

void QmitkCloseTool3DGUI::OnAcceptPreview()
{
  if (m_CloseTool3D.IsNotNull())
  {
    m_CloseTool3D->AcceptPreview();
  }
}

void QmitkCloseTool3DGUI::OnCalculateDifference()
{
  if (m_CloseTool3D.IsNotNull())
  {
    m_CloseTool3D->CalculateDifference();
  }
}

void QmitkCloseTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

void QmitkCloseTool3DGUI::OnShowInformation( bool on )
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

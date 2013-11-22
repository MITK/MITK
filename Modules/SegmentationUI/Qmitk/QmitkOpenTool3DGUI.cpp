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

#include "QmitkOpenTool3DGUI.h"

#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkOpenTool3DGUI, "")

QmitkOpenTool3DGUI::QmitkOpenTool3DGUI() : QmitkToolGUI()
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbDifference, SIGNAL(clicked()), this, SLOT(OnCalculateDifference()) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkOpenTool3DGUI::~QmitkOpenTool3DGUI()
{
  if (m_OpenTool3D.IsNotNull())
  {
    m_OpenTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkOpenTool3DGUI, bool>( this, &QmitkOpenTool3DGUI::BusyStateChanged );
  }
}

void QmitkOpenTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_OpenTool3D.IsNotNull())
  {
    m_OpenTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkOpenTool3DGUI, bool>( this, &QmitkOpenTool3DGUI::BusyStateChanged );
  }

  m_OpenTool3D = dynamic_cast<mitk::OpenTool3D*>( tool );

  if (m_OpenTool3D.IsNotNull())
  {
    m_OpenTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkOpenTool3DGUI, bool>( this, &QmitkOpenTool3DGUI::BusyStateChanged );
  }
}

void QmitkOpenTool3DGUI::OnRun()
{
  if (m_OpenTool3D.IsNotNull())
  {
    m_OpenTool3D->Run();
  }
}

void QmitkOpenTool3DGUI::OnAcceptPreview()
{
  if (m_OpenTool3D.IsNotNull())
  {
    m_OpenTool3D->AcceptPreview();
  }
}

void QmitkOpenTool3DGUI::OnCalculateDifference()
{
  if (m_OpenTool3D.IsNotNull())
  {
    m_OpenTool3D->CalculateDifference();
  }
}

void QmitkOpenTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

void QmitkOpenTool3DGUI::OnShowInformation( bool on )
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

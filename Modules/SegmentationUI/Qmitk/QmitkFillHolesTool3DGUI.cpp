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

#include "QmitkFillHolesTool3DGUI.h"

#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkFillHolesTool3DGUI, "")

QmitkFillHolesTool3DGUI::QmitkFillHolesTool3DGUI() : QmitkToolGUI()
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbDifference, SIGNAL(clicked()), this, SLOT(OnCalculateDifference()) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkFillHolesTool3DGUI::~QmitkFillHolesTool3DGUI()
{
  if (m_FillHolesTool3D.IsNotNull())
  {
    m_FillHolesTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFillHolesTool3DGUI, bool>( this, &QmitkFillHolesTool3DGUI::BusyStateChanged );
  }
}

void QmitkFillHolesTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_FillHolesTool3D.IsNotNull())
  {
    m_FillHolesTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFillHolesTool3DGUI, bool>( this, &QmitkFillHolesTool3DGUI::BusyStateChanged );
  }

  m_FillHolesTool3D = dynamic_cast<mitk::FillHolesTool3D*>( tool );

  if (m_FillHolesTool3D.IsNotNull())
  {
    m_FillHolesTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkFillHolesTool3DGUI, bool>( this, &QmitkFillHolesTool3DGUI::BusyStateChanged );
  }
}

void QmitkFillHolesTool3DGUI::OnRun()
{
  if (m_FillHolesTool3D.IsNotNull())
  {
    m_FillHolesTool3D->Run();
  }
}

void QmitkFillHolesTool3DGUI::OnAcceptPreview()
{
  if (m_FillHolesTool3D.IsNotNull())
  {
    m_FillHolesTool3D->AcceptPreview();
  }
}

void QmitkFillHolesTool3DGUI::OnCalculateDifference()
{
  if (m_FillHolesTool3D.IsNotNull())
  {
    m_FillHolesTool3D->CalculateDifference();
  }
}

void QmitkFillHolesTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

void QmitkFillHolesTool3DGUI::OnShowInformation( bool on )
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

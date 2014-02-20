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

#include <mitkFillHolesTool3D.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkFillHolesTool3DGUI, "")

QmitkFillHolesTool3DGUI::QmitkFillHolesTool3DGUI() : QmitkToolGUI(), m_FillHolesTool3D(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbDifference, SIGNAL(clicked()), this, SLOT(OnInvertPreview()) );
  connect( m_Controls.m_pbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbShowAdvancedTools, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkFillHolesTool3DGUI::~QmitkFillHolesTool3DGUI()
{
  if (m_FillHolesTool3D)
  {
    m_FillHolesTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFillHolesTool3DGUI, bool>( this, &QmitkFillHolesTool3DGUI::BusyStateChanged );
  }
}

void QmitkFillHolesTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_FillHolesTool3D)
  {
    m_FillHolesTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFillHolesTool3DGUI, bool>( this, &QmitkFillHolesTool3DGUI::BusyStateChanged );
  }

  m_FillHolesTool3D = dynamic_cast<mitk::FillHolesTool3D*>( tool );

  if (m_FillHolesTool3D)
  {
    m_FillHolesTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkFillHolesTool3DGUI, bool>( this, &QmitkFillHolesTool3DGUI::BusyStateChanged );
  }
}

void QmitkFillHolesTool3DGUI::OnRun()
{
  if (m_FillHolesTool3D)
  {
    m_FillHolesTool3D->SetConsiderAllLabels(m_Controls.m_rbConsiderAllLabels->isChecked());
    m_FillHolesTool3D->Run();
  }
}

void QmitkFillHolesTool3DGUI::OnCancel()
{
  if (m_FillHolesTool3D)
  {
    m_FillHolesTool3D->Cancel();
  }
}

void QmitkFillHolesTool3DGUI::OnAcceptPreview()
{
  if (m_FillHolesTool3D)
  {
    m_FillHolesTool3D->AcceptPreview();
  }
}

void QmitkFillHolesTool3DGUI::OnInvertPreview()
{
  if (m_FillHolesTool3D)
  {
    m_FillHolesTool3D->InvertPreview();
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

void QmitkFillHolesTool3DGUI::OnShowAdvancedControls( bool on )
{
  if (on)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

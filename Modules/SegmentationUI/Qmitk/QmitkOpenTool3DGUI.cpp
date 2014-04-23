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

#include <mitkOpenTool3D.h>
#include <QmitkNewSegmentationDialog.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(MitkSegmentationUI_EXPORT, QmitkOpenTool3DGUI, "")

QmitkOpenTool3DGUI::QmitkOpenTool3DGUI() : QmitkToolGUI(), m_OpenTool3D(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbDifference, SIGNAL(clicked()), this, SLOT(OnInvertPreview()) );
  connect( m_Controls.m_sbKernelSize, SIGNAL(valueChanged(int)), this, SLOT(OnKernelSizeChanged(int)) );
  connect( m_Controls.m_pbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_pbShowAdvancedTools, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkOpenTool3DGUI::~QmitkOpenTool3DGUI()
{
  if (m_OpenTool3D)
  {
    m_OpenTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkOpenTool3DGUI, bool>( this, &QmitkOpenTool3DGUI::BusyStateChanged );
  }
}

void QmitkOpenTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_OpenTool3D)
  {
    m_OpenTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkOpenTool3DGUI, bool>( this, &QmitkOpenTool3DGUI::BusyStateChanged );
  }

  m_OpenTool3D = dynamic_cast<mitk::OpenTool3D*>( tool );

  if (m_OpenTool3D)
  {
    m_Controls.m_sbKernelSize->setValue( m_OpenTool3D->GetRadius() );
    m_OpenTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkOpenTool3DGUI, bool>( this, &QmitkOpenTool3DGUI::BusyStateChanged );
  }
}

void QmitkOpenTool3DGUI::OnRun()
{
  if (m_OpenTool3D)
  {
    m_OpenTool3D->SetRadius(m_Controls.m_sbKernelSize->value());
    m_OpenTool3D->Run();
  }
}

void QmitkOpenTool3DGUI::OnCancel()
{
  if (m_OpenTool3D)
  {
    m_OpenTool3D->Cancel();
  }
}

void QmitkOpenTool3DGUI::OnKernelSizeChanged(int value)
{
  if (m_OpenTool3D)
  {
    m_OpenTool3D->SetRadius(value);
  }
}

void QmitkOpenTool3DGUI::OnAcceptPreview()
{
  if (m_OpenTool3D)
  {
    m_OpenTool3D->AcceptPreview();
  }
}

void QmitkOpenTool3DGUI::OnInvertPreview()
{
  if (m_OpenTool3D)
  {
    m_OpenTool3D->InvertPreview();
  }
}

void QmitkOpenTool3DGUI::OnNewLabel()
{
  if (m_OpenTool3D)
  {
    QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this );
//    dialog->SetSuggestionList( m_OrganColors );
    dialog->setWindowTitle("New Label");

    int dialogReturnValue = dialog->exec();

    if ( dialogReturnValue == QDialog::Rejected ) return;

    mitk::Color color = dialog->GetColor();

    m_OpenTool3D->CreateNewLabel(dialog->GetSegmentationName().toStdString(), color);
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

void QmitkOpenTool3DGUI::OnShowAdvancedControls( bool on )
{
  if (on)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

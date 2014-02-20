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

#include <mitkCloseTool3D.h>
#include <QmitkNewSegmentationDialog.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkCloseTool3DGUI, "")

QmitkCloseTool3DGUI::QmitkCloseTool3DGUI() : QmitkToolGUI(), m_CloseTool3D(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbInvertPreview, SIGNAL(clicked()), this, SLOT(OnInvertPreview()) );
  connect( m_Controls.m_sbKernelSize, SIGNAL(valueChanged(int)), this, SLOT(OnKernelSizeChanged(int)) );
  connect( m_Controls.m_pbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_pbShowAdvancedTools, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkCloseTool3DGUI::~QmitkCloseTool3DGUI()
{
  if (m_CloseTool3D)
  {
    m_CloseTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkCloseTool3DGUI, bool>( this, &QmitkCloseTool3DGUI::BusyStateChanged );
  }
}

void QmitkCloseTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_CloseTool3D)
  {
    m_CloseTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkCloseTool3DGUI, bool>( this, &QmitkCloseTool3DGUI::BusyStateChanged );
  }

  m_CloseTool3D = dynamic_cast<mitk::CloseTool3D*>( tool );

  if (m_CloseTool3D)
  {
    m_Controls.m_sbKernelSize->setValue( m_CloseTool3D->GetRadius() );
    m_CloseTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkCloseTool3DGUI, bool>( this, &QmitkCloseTool3DGUI::BusyStateChanged );
  }
}

void QmitkCloseTool3DGUI::OnRun()
{
  if (m_CloseTool3D)
  {
    m_CloseTool3D->SetRadius(m_Controls.m_sbKernelSize->value());
    m_CloseTool3D->Run();
  }
}

void QmitkCloseTool3DGUI::OnCancel()
{
  if (m_CloseTool3D)
  {
    m_CloseTool3D->Cancel();
  }
}

void QmitkCloseTool3DGUI::OnKernelSizeChanged(int value)
{
  if (m_CloseTool3D)
  {
    m_CloseTool3D->SetRadius(value);
  }
}

void QmitkCloseTool3DGUI::OnAcceptPreview()
{
  if (m_CloseTool3D)
  {
    m_CloseTool3D->AcceptPreview();
  }
}

void QmitkCloseTool3DGUI::OnInvertPreview()
{
  if (m_CloseTool3D)
  {
    m_CloseTool3D->InvertPreview();
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

void QmitkCloseTool3DGUI::OnShowAdvancedControls( bool on )
{
  if (on)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

void QmitkCloseTool3DGUI::OnNewLabel()
{
  if (m_CloseTool3D)
  {
    QmitkNewSegmentationDialog dialog(this);
//    dialog->SetSuggestionList( m_OrganColors );
    dialog.setWindowTitle("New Label");
    int dialogReturnValue = dialog.exec();
    if ( dialogReturnValue == QDialog::Rejected ) return;
    mitk::Color color = dialog.GetColor();
    std::string name = dialog.GetSegmentationName().toStdString();
    m_CloseTool3D->CreateNewLabel(name, color);
  }
}

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

#include "QmitkAutoThresholdTool3DGUI.h"

#include <QmitkNewSegmentationDialog.h>
#include "mitkAutoThresholdTool3D.h"

#include <QApplication.h>

#include <QMessageBox>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkAutoThresholdTool3DGUI, "")

QmitkAutoThresholdTool3DGUI::QmitkAutoThresholdTool3DGUI() : QmitkToolGUI(), m_AutoThresholdTool3D(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbInvertPreview, SIGNAL(clicked()), this, SLOT(OnInvertPreview()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbShowAdvancedTools, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( m_Controls.m_cbMethod, SIGNAL(currentIndexChanged (int)), this, SLOT(OnMethodChanged(int)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkAutoThresholdTool3DGUI::~QmitkAutoThresholdTool3DGUI()
{
  if (m_AutoThresholdTool3D)
  {
    m_AutoThresholdTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkAutoThresholdTool3DGUI, bool>( this, &QmitkAutoThresholdTool3DGUI::BusyStateChanged );
  }
}

void QmitkAutoThresholdTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_AutoThresholdTool3D)
  {
    m_AutoThresholdTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkAutoThresholdTool3DGUI, bool>( this, &QmitkAutoThresholdTool3DGUI::BusyStateChanged );
  }

  m_AutoThresholdTool3D = dynamic_cast<mitk::AutoThresholdTool3D*>( tool );

  if (m_AutoThresholdTool3D)
  {
    m_AutoThresholdTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkAutoThresholdTool3DGUI, bool>( this, &QmitkAutoThresholdTool3DGUI::BusyStateChanged );
  }
}

void QmitkAutoThresholdTool3DGUI::OnMethodChanged(int index)
{
  if (m_AutoThresholdTool3D)
  {
    m_AutoThresholdTool3D->SetMethod(static_cast<mitk::AutoThresholdTool3D::AutoThresholdType>(index));
  }
}

void QmitkAutoThresholdTool3DGUI::OnAcceptPreview()
{
  if (m_AutoThresholdTool3D)
  {
    QmitkNewSegmentationDialog dialog(this);
//    dialog->SetSuggestionList( m_OrganColors );
    dialog.setWindowTitle("New Label");
    int dialogReturnValue = dialog.exec();
    if ( dialogReturnValue == QDialog::Rejected ) return;
    mitk::Color color = dialog.GetColor();
    std::string name = dialog.GetSegmentationName().toStdString();
    m_AutoThresholdTool3D->CreateNewLabel(name, color);
  }
}

void QmitkAutoThresholdTool3DGUI::OnInvertPreview()
{
  if (m_AutoThresholdTool3D)
  {
    m_AutoThresholdTool3D->InvertPreview();
  }
}

void QmitkAutoThresholdTool3DGUI::OnRun()
{
  if (m_AutoThresholdTool3D)
  {
    m_AutoThresholdTool3D->Run();
  }
}

void QmitkAutoThresholdTool3DGUI::OnCancel()
{
  if (m_AutoThresholdTool3D)
  {
    m_AutoThresholdTool3D->Cancel();
  }
}

void QmitkAutoThresholdTool3DGUI::OnShowInformation( bool value )
{
  if (value)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkAutoThresholdTool3DGUI::OnShowAdvancedControls( bool value )
{
  if (value)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

void QmitkAutoThresholdTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

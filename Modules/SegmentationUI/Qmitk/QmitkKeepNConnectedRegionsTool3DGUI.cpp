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

#include "QmitkKeepNConnectedRegionsTool3DGUI.h"

#include "mitkKeepNConnectedRegionsTool3D.h"

#include <QmitkNewSegmentationDialog.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(MitkSegmentationUI_EXPORT, QmitkKeepNConnectedRegionsTool3DGUI, "")

QmitkKeepNConnectedRegionsTool3DGUI::QmitkKeepNConnectedRegionsTool3DGUI() : QmitkToolGUI(), m_KeepNConnectedRegionsTool3D(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbDifference, SIGNAL(clicked()), this, SLOT(OnInvertPreview()) );
  connect( m_Controls.m_sbConnectedRegionsToKeep, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfConnectedRegionsToKeepChanged(int)) );
  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_pbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbShowAdvancedTools, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkKeepNConnectedRegionsTool3DGUI::~QmitkKeepNConnectedRegionsTool3DGUI()
{
  if (m_KeepNConnectedRegionsTool3D)
  {
    m_KeepNConnectedRegionsTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkKeepNConnectedRegionsTool3DGUI, bool>( this, &QmitkKeepNConnectedRegionsTool3DGUI::BusyStateChanged );
  }
}

void QmitkKeepNConnectedRegionsTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_KeepNConnectedRegionsTool3D)
  {
    m_KeepNConnectedRegionsTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkKeepNConnectedRegionsTool3DGUI, bool>( this, &QmitkKeepNConnectedRegionsTool3DGUI::BusyStateChanged );
  }

  m_KeepNConnectedRegionsTool3D = dynamic_cast<mitk::KeepNConnectedRegionsTool3D*>( tool );

  if (m_KeepNConnectedRegionsTool3D)
  {
    m_KeepNConnectedRegionsTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkKeepNConnectedRegionsTool3DGUI, bool>( this, &QmitkKeepNConnectedRegionsTool3DGUI::BusyStateChanged );
  }
}

void QmitkKeepNConnectedRegionsTool3DGUI::OnRun()
{
  if (m_KeepNConnectedRegionsTool3D)
  {
    m_KeepNConnectedRegionsTool3D->SetNumberOfConnectedRegionsToKeep(m_Controls.m_sbConnectedRegionsToKeep->value());
    m_KeepNConnectedRegionsTool3D->Run();
  }
}

void QmitkKeepNConnectedRegionsTool3DGUI::OnCancel()
{
  if (m_KeepNConnectedRegionsTool3D)
  {
    m_KeepNConnectedRegionsTool3D->Cancel();
  }
}

void QmitkKeepNConnectedRegionsTool3DGUI::OnNumberOfConnectedRegionsToKeepChanged(int value)
{
  if (m_KeepNConnectedRegionsTool3D)
  {
    m_KeepNConnectedRegionsTool3D->SetNumberOfConnectedRegionsToKeep(value);
  }
}

void QmitkKeepNConnectedRegionsTool3DGUI::OnAcceptPreview()
{
  if (m_KeepNConnectedRegionsTool3D)
  {
    m_KeepNConnectedRegionsTool3D->AcceptPreview();
  }
}

void QmitkKeepNConnectedRegionsTool3DGUI::OnInvertPreview()
{
  if (m_KeepNConnectedRegionsTool3D)
  {
    m_KeepNConnectedRegionsTool3D->InvertPreview();
  }
}

void QmitkKeepNConnectedRegionsTool3DGUI::OnCalculateUnion()
{
  if (m_KeepNConnectedRegionsTool3D)
  {
    m_KeepNConnectedRegionsTool3D->AddPreview();
  }
}

void QmitkKeepNConnectedRegionsTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

void QmitkKeepNConnectedRegionsTool3DGUI::OnShowInformation( bool on )
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkKeepNConnectedRegionsTool3DGUI::OnShowAdvancedControls( bool on )
{
  if (on)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

void QmitkKeepNConnectedRegionsTool3DGUI::OnNewLabel()
{
  if (m_KeepNConnectedRegionsTool3D)
  {
    QmitkNewSegmentationDialog dialog(this);
//    dialog->SetSuggestionList( m_OrganColors );
    dialog.setWindowTitle("New Label");
    int dialogReturnValue = dialog.exec();
    if ( dialogReturnValue == QDialog::Rejected ) return;
    mitk::Color color = dialog.GetColor();
    std::string name = dialog.GetSegmentationName().toStdString();
    m_KeepNConnectedRegionsTool3D->CreateNewLabel(name, color);
  }
}

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

#include "QmitkSplitConnectedRegionsTool3DGUI.h"

#include <QmitkNewSegmentationDialog.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkSplitConnectedRegionsTool3DGUI, "")

QmitkSplitConnectedRegionsTool3DGUI::QmitkSplitConnectedRegionsTool3DGUI() : QmitkToolGUI(), m_SplitConnectedRegionsTool3D(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbInvertPreview, SIGNAL(clicked()), this, SLOT(OnInvertPreview()) );
  connect( m_Controls.m_sbConnectedRegionsToKeep, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfConnectedRegionsToKeepChanged(int)) );
  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_cbShowAdvancedControls, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkSplitConnectedRegionsTool3DGUI::~QmitkSplitConnectedRegionsTool3DGUI()
{
  if (m_SplitConnectedRegionsTool3D)
  {
    m_SplitConnectedRegionsTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkSplitConnectedRegionsTool3DGUI, bool>( this, &QmitkSplitConnectedRegionsTool3DGUI::BusyStateChanged );
  }
}

void QmitkSplitConnectedRegionsTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_SplitConnectedRegionsTool3D)
  {
    m_SplitConnectedRegionsTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkSplitConnectedRegionsTool3DGUI, bool>( this, &QmitkSplitConnectedRegionsTool3DGUI::BusyStateChanged );
  }

  m_SplitConnectedRegionsTool3D = dynamic_cast<mitk::SplitConnectedRegionsTool3D*>( tool );

  if (m_SplitConnectedRegionsTool3D)
  {
    m_SplitConnectedRegionsTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkSplitConnectedRegionsTool3DGUI, bool>( this, &QmitkSplitConnectedRegionsTool3DGUI::BusyStateChanged );
  }
}

void QmitkSplitConnectedRegionsTool3DGUI::OnRun()
{
  if (m_SplitConnectedRegionsTool3D)
  {
    m_SplitConnectedRegionsTool3D->SetNumberOfConnectedRegionsToKeep(m_Controls.m_sbConnectedRegionsToKeep->value());
    m_SplitConnectedRegionsTool3D->Run();
  }
}

void QmitkSplitConnectedRegionsTool3DGUI::OnCancel()
{
  if (m_SplitConnectedRegionsTool3D)
  {
    m_SplitConnectedRegionsTool3D->Cancel();
  }
}

void QmitkSplitConnectedRegionsTool3DGUI::OnNumberOfConnectedRegionsToKeepChanged(int value)
{
  if (m_SplitConnectedRegionsTool3D)
  {
    m_SplitConnectedRegionsTool3D->SetNumberOfConnectedRegionsToKeep(value);
  }
}

void QmitkSplitConnectedRegionsTool3DGUI::OnAcceptPreview()
{
  if (m_SplitConnectedRegionsTool3D)
  {
    m_SplitConnectedRegionsTool3D->AcceptPreview();
  }
}

void QmitkSplitConnectedRegionsTool3DGUI::OnInvertPreview()
{
  if (m_SplitConnectedRegionsTool3D)
  {
    m_SplitConnectedRegionsTool3D->InvertPreview();
  }
}

void QmitkSplitConnectedRegionsTool3DGUI::OnCalculateUnion()
{
  if (m_SplitConnectedRegionsTool3D)
  {
    m_SplitConnectedRegionsTool3D->AddPreview();
  }
}

void QmitkSplitConnectedRegionsTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

void QmitkSplitConnectedRegionsTool3DGUI::OnShowInformation( bool on )
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkSplitConnectedRegionsTool3DGUI::OnShowAdvancedControls( bool on )
{
  if (on)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

void QmitkSplitConnectedRegionsTool3DGUI::OnNewLabel()
{
  if (m_SplitConnectedRegionsTool3D)
  {
    QmitkNewSegmentationDialog dialog(this);
//    dialog->SetSuggestionList( m_OrganColors );
    dialog.setWindowTitle("New Label");
    int dialogReturnValue = dialog.exec();
    if ( dialogReturnValue == QDialog::Rejected ) return;
    mitk::Color color = dialog.GetColor();
    std::string name = dialog.GetSegmentationName().toStdString();
    m_SplitConnectedRegionsTool3D->CreateNewLabel(name, color);
  }
}

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

#include "QmitkConnectedRegionSelectorTool3DGUI.h"

#include <mitkConnectedRegionSelectorTool3D.h>
#include <QmitkNewSegmentationDialog.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkConnectedRegionSelectorTool3DGUI, "")

QmitkConnectedRegionSelectorTool3DGUI::QmitkConnectedRegionSelectorTool3DGUI() : QmitkToolGUI(), m_ConnectedRegionSelectorTool3D(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbInvertPreview, SIGNAL(clicked()), this, SLOT(OnInvertPreview()) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_cbShowAdvancedControls, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkConnectedRegionSelectorTool3DGUI::~QmitkConnectedRegionSelectorTool3DGUI()
{
  if (m_ConnectedRegionSelectorTool3D)
  {
    m_ConnectedRegionSelectorTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkConnectedRegionSelectorTool3DGUI, bool>( this, &QmitkConnectedRegionSelectorTool3DGUI::BusyStateChanged );
  }
}

void QmitkConnectedRegionSelectorTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_ConnectedRegionSelectorTool3D)
  {
    m_ConnectedRegionSelectorTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkConnectedRegionSelectorTool3DGUI, bool>( this, &QmitkConnectedRegionSelectorTool3DGUI::BusyStateChanged );
  }

  m_ConnectedRegionSelectorTool3D = dynamic_cast<mitk::ConnectedRegionSelectorTool3D*>( tool );

  if (m_ConnectedRegionSelectorTool3D)
  {
    m_ConnectedRegionSelectorTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkConnectedRegionSelectorTool3DGUI, bool>( this, &QmitkConnectedRegionSelectorTool3DGUI::BusyStateChanged );
  }
}

void QmitkConnectedRegionSelectorTool3DGUI::OnCancel()
{
  if (m_ConnectedRegionSelectorTool3D)
  {
    m_ConnectedRegionSelectorTool3D->Cancel();
  }
}

void QmitkConnectedRegionSelectorTool3DGUI::OnAcceptPreview()
{
  if (m_ConnectedRegionSelectorTool3D)
  {
    m_ConnectedRegionSelectorTool3D->AcceptPreview();
  }
}

void QmitkConnectedRegionSelectorTool3DGUI::OnInvertPreview()
{
  if (m_ConnectedRegionSelectorTool3D)
  {
    m_ConnectedRegionSelectorTool3D->InvertPreview();
  }
}

void QmitkConnectedRegionSelectorTool3DGUI::OnNewLabel()
{
  if (m_ConnectedRegionSelectorTool3D)
  {
    QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this );
//    dialog->SetSuggestionList( m_OrganColors );
    dialog->setWindowTitle("New Label");

    int dialogReturnValue = dialog->exec();

    if ( dialogReturnValue == QDialog::Rejected ) return;

    mitk::Color color = dialog->GetColor();

    m_ConnectedRegionSelectorTool3D->CreateNewLabel(dialog->GetSegmentationName().toStdString(), color);
  }
}

void QmitkConnectedRegionSelectorTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

void QmitkConnectedRegionSelectorTool3DGUI::OnShowInformation( bool on )
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkConnectedRegionSelectorTool3DGUI::OnShowAdvancedControls( bool on )
{
  if (on)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

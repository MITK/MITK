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

#include "QmitkTriangleThresholdingTool3DGUI.h"

#include <QmitkNewSegmentationDialog.h>
#include "mitkTriangleThresholdingTool3D.h"

#include <QApplication.h>

#include <QMessageBox>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkTriangleThresholdingTool3DGUI, "")

QmitkTriangleThresholdingTool3DGUI::QmitkTriangleThresholdingTool3DGUI() : QmitkToolGUI(), m_TriangleThresholdingTool3D(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbInvertPreview, SIGNAL(clicked()), this, SLOT(OnInvertPreview()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_cbShowAdvancedControls, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkTriangleThresholdingTool3DGUI::~QmitkTriangleThresholdingTool3DGUI()
{
  if (m_TriangleThresholdingTool3D)
  {
    m_TriangleThresholdingTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkTriangleThresholdingTool3DGUI, bool>( this, &QmitkTriangleThresholdingTool3DGUI::BusyStateChanged );
  }
}

void QmitkTriangleThresholdingTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_TriangleThresholdingTool3D)
  {
    m_TriangleThresholdingTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkTriangleThresholdingTool3DGUI, bool>( this, &QmitkTriangleThresholdingTool3DGUI::BusyStateChanged );
  }

  m_TriangleThresholdingTool3D = dynamic_cast<mitk::TriangleThresholdingTool3D*>( tool );

  if (m_TriangleThresholdingTool3D)
  {
    m_TriangleThresholdingTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkTriangleThresholdingTool3DGUI, bool>( this, &QmitkTriangleThresholdingTool3DGUI::BusyStateChanged );
  }
}

void QmitkTriangleThresholdingTool3DGUI::OnAcceptPreview()
{
  if (m_TriangleThresholdingTool3D)
  {
    QmitkNewSegmentationDialog dialog(this);
//    dialog->SetSuggestionList( m_OrganColors );
    dialog.setWindowTitle("New Label");
    int dialogReturnValue = dialog.exec();
    if ( dialogReturnValue == QDialog::Rejected ) return;
    mitk::Color color = dialog.GetColor();
    std::string name = dialog.GetSegmentationName().toStdString();
    m_TriangleThresholdingTool3D->CreateNewLabel(name, color);
  }
}

void QmitkTriangleThresholdingTool3DGUI::OnInvertPreview()
{
  if (m_TriangleThresholdingTool3D)
  {
    m_TriangleThresholdingTool3D->InvertPreview();
  }
}

void QmitkTriangleThresholdingTool3DGUI::OnRun()
{
  if (m_TriangleThresholdingTool3D)
  {
    m_TriangleThresholdingTool3D->Run();
  }
}

void QmitkTriangleThresholdingTool3DGUI::OnCancel()
{
  if (m_TriangleThresholdingTool3D)
  {
    m_TriangleThresholdingTool3D->Cancel();
  }
}

void QmitkTriangleThresholdingTool3DGUI::OnShowInformation( bool value )
{
  if (value)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkTriangleThresholdingTool3DGUI::OnShowAdvancedControls( bool value )
{
  if (value)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

void QmitkTriangleThresholdingTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

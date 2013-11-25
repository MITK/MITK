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

#include "QmitkErodeTool3DGUI.h"

#include "QmitkNewSegmentationDialog.h"
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkErodeTool3DGUI, "")

QmitkErodeTool3DGUI::QmitkErodeTool3DGUI() : QmitkToolGUI()
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbDifference, SIGNAL(clicked()), this, SLOT(OnCalculateDifference()) );
  connect( m_Controls.m_sbKernelSize, SIGNAL(valueChanged(int)), this, SLOT(OnKernelSizeChanged(int)) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_cbShowAdvancedControls, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkErodeTool3DGUI::~QmitkErodeTool3DGUI()
{
  if (m_ErodeTool3D.IsNotNull())
  {
    m_ErodeTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkErodeTool3DGUI, bool>( this, &QmitkErodeTool3DGUI::BusyStateChanged );
  }
}

void QmitkErodeTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_ErodeTool3D.IsNotNull())
  {
    m_ErodeTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkErodeTool3DGUI, bool>( this, &QmitkErodeTool3DGUI::BusyStateChanged );
  }

  m_ErodeTool3D = dynamic_cast<mitk::ErodeTool3D*>( tool );

  if (m_ErodeTool3D.IsNotNull())
  {
    m_ErodeTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkErodeTool3DGUI, bool>( this, &QmitkErodeTool3DGUI::BusyStateChanged );
  }
}

void QmitkErodeTool3DGUI::OnRun()
{
  if (m_ErodeTool3D.IsNotNull())
  {
    m_ErodeTool3D->Run();
  }
}

void QmitkErodeTool3DGUI::OnKernelSizeChanged(int value)
{
  if (m_ErodeTool3D.IsNotNull())
  {
    m_ErodeTool3D->SetRadius(value);
  }
}

void QmitkErodeTool3DGUI::OnAcceptPreview()
{
  if (m_ErodeTool3D.IsNotNull())
  {
    m_ErodeTool3D->AcceptPreview();
  }
}

void QmitkErodeTool3DGUI::OnCalculateDifference()
{
  if (m_ErodeTool3D.IsNotNull())
  {
    m_ErodeTool3D->CalculateDifference();
  }
}

void QmitkErodeTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

void QmitkErodeTool3DGUI::OnShowInformation( bool on )
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkErodeTool3DGUI::OnShowAdvancedControls( bool on )
{
  if (on)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

void QmitkErodeTool3DGUI::OnNewLabel()
{
  if (m_ErodeTool3D.IsNotNull())
  {
    QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this );
//    dialog->SetSuggestionList( m_OrganColors );
    dialog->setWindowTitle("New Label");

    int dialogReturnValue = dialog->exec();

    if ( dialogReturnValue == QDialog::Rejected ) return;

    mitk::Color color = dialog->GetColor();

    m_ErodeTool3D->CreateNewLabel(dialog->GetSegmentationName().toStdString(), color);
  }
}

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

#include "QmitkDilateTool3DGUI.h"
#include "QmitkNewSegmentationDialog.h"
#include "QmitkConfirmSegmentationDialog.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkDilateTool3DGUI, "")

QmitkDilateTool3DGUI::QmitkDilateTool3DGUI() : QmitkToolGUI()
{
  // create the visible widgets
  QBoxLayout* mainLayout = new QVBoxLayout(this);

  QPushButton* btRun = new QPushButton("Run", this);
  connect( btRun, SIGNAL(clicked()), this, SLOT(OnRun()));
  mainLayout->addWidget( btRun );

 connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkDilateTool3DGUI::~QmitkDilateTool3DGUI()
{
  if (m_DilateTool3D.IsNotNull())
  {
    m_DilateTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkDilateTool3DGUI, bool>( this, &QmitkDilateTool3DGUI::BusyStateChanged );
  }
}

void QmitkDilateTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_DilateTool3D.IsNotNull())
  {
    m_DilateTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkDilateTool3DGUI, bool>( this, &QmitkDilateTool3DGUI::BusyStateChanged );
  }

  m_DilateTool3D = dynamic_cast<mitk::DilateTool3D*>( tool );

  if (m_DilateTool3D.IsNotNull())
  {
    m_DilateTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkDilateTool3DGUI, bool>( this, &QmitkDilateTool3DGUI::BusyStateChanged );
  }
}

void QmitkDilateTool3DGUI::OnRun()
{
  if (m_DilateTool3D.IsNotNull())
  {
    m_DilateTool3D->Run();
  }
}

void QmitkDilateTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

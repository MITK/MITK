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

#include "QmitkSmoothTool3DGUI.h"
#include "QmitkNewSegmentationDialog.h"
#include "QmitkConfirmSegmentationDialog.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkSmoothTool3DGUI, "")

QmitkSmoothTool3DGUI::QmitkSmoothTool3DGUI() : QmitkToolGUI()
{
  // create the visible widgets
  QBoxLayout* mainLayout = new QVBoxLayout(this);

  QPushButton* btRun = new QPushButton("Run", this);
  connect( btRun, SIGNAL(clicked()), this, SLOT(OnRun()));
  mainLayout->addWidget( btRun );

 connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkSmoothTool3DGUI::~QmitkSmoothTool3DGUI()
{
  if (m_SmoothTool3D.IsNotNull())
  {
    m_SmoothTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkSmoothTool3DGUI, bool>( this, &QmitkSmoothTool3DGUI::BusyStateChanged );
  }
}

void QmitkSmoothTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_SmoothTool3D.IsNotNull())
  {
    m_SmoothTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkSmoothTool3DGUI, bool>( this, &QmitkSmoothTool3DGUI::BusyStateChanged );
  }

  m_SmoothTool3D = dynamic_cast<mitk::SmoothTool3D*>( tool );

  if (m_SmoothTool3D.IsNotNull())
  {
    m_SmoothTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkSmoothTool3DGUI, bool>( this, &QmitkSmoothTool3DGUI::BusyStateChanged );
  }
}

void QmitkSmoothTool3DGUI::OnRun()
{
  if (m_SmoothTool3D.IsNotNull())
  {
    m_SmoothTool3D->Run();
  }
}

void QmitkSmoothTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

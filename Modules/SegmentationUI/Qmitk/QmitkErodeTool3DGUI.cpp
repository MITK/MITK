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
#include "QmitkConfirmSegmentationDialog.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkErodeTool3DGUI, "")

QmitkErodeTool3DGUI::QmitkErodeTool3DGUI() : QmitkToolGUI()
{
  // create the visible widgets
  QBoxLayout* mainLayout = new QVBoxLayout(this);

  QPushButton* btRun = new QPushButton("Run", this);
  connect( btRun, SIGNAL(clicked()), this, SLOT(OnRun()));
  mainLayout->addWidget( btRun );

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

void QmitkErodeTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

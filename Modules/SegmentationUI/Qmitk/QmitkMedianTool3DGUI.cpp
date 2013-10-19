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

#include "QmitkMedianTool3DGUI.h"
#include "QmitkNewSegmentationDialog.h"
#include "QmitkConfirmSegmentationDialog.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkMedianTool3DGUI, "")

QmitkMedianTool3DGUI::QmitkMedianTool3DGUI() : QmitkToolGUI()
{
  // create the visible widgets
  QBoxLayout* mainLayout = new QVBoxLayout(this);

  QPushButton* btRun = new QPushButton("Run", this);
  connect( btRun, SIGNAL(clicked()), this, SLOT(OnRun()));
  mainLayout->addWidget( btRun );

 connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkMedianTool3DGUI::~QmitkMedianTool3DGUI()
{
  if (m_MedianTool3D.IsNotNull())
  {
    m_MedianTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkMedianTool3DGUI, bool>( this, &QmitkMedianTool3DGUI::BusyStateChanged );
  }
}

void QmitkMedianTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_MedianTool3D.IsNotNull())
  {
    m_MedianTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkMedianTool3DGUI, bool>( this, &QmitkMedianTool3DGUI::BusyStateChanged );
  }

  m_MedianTool3D = dynamic_cast<mitk::MedianTool3D*>( tool );

  if (m_MedianTool3D.IsNotNull())
  {
    m_MedianTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkMedianTool3DGUI, bool>( this, &QmitkMedianTool3DGUI::BusyStateChanged );
  }
}

void QmitkMedianTool3DGUI::OnRun()
{
  if (m_MedianTool3D.IsNotNull())
  {
    m_MedianTool3D->Run();
  }
}

void QmitkMedianTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

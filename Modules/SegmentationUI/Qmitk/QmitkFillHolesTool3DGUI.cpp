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

#include "QmitkFillHolesTool3DGUI.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkFillHolesTool3DGUI, "")

QmitkFillHolesTool3DGUI::QmitkFillHolesTool3DGUI() : QmitkToolGUI()
{
  // create the visible widgets
  QBoxLayout* mainLayout = new QVBoxLayout(this);

  QPushButton* btRun = new QPushButton("Run", this);
  connect( btRun, SIGNAL(clicked()), this, SLOT(OnRun()));
  mainLayout->addWidget( btRun );

 connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkFillHolesTool3DGUI::~QmitkFillHolesTool3DGUI()
{
  if (m_FillHolesTool3D.IsNotNull())
  {
    m_FillHolesTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFillHolesTool3DGUI, bool>( this, &QmitkFillHolesTool3DGUI::BusyStateChanged );
  }
}

void QmitkFillHolesTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_FillHolesTool3D.IsNotNull())
  {
    m_FillHolesTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFillHolesTool3DGUI, bool>( this, &QmitkFillHolesTool3DGUI::BusyStateChanged );
  }

  m_FillHolesTool3D = dynamic_cast<mitk::FillHolesTool3D*>( tool );

  if (m_FillHolesTool3D.IsNotNull())
  {
    m_FillHolesTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkFillHolesTool3DGUI, bool>( this, &QmitkFillHolesTool3DGUI::BusyStateChanged );
  }
}

void QmitkFillHolesTool3DGUI::OnRun()
{
  if (m_FillHolesTool3D.IsNotNull())
  {
    m_FillHolesTool3D->Run();
  }
}

void QmitkFillHolesTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

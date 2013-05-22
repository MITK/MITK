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

#include "QmitkOtsuTool3DGUI.h"
#include "QmitkNewSegmentationDialog.h"

#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qlayout.h>

MITK_TOOL_GUI_MACRO(QmitkExt_EXPORT, QmitkOtsuTool3DGUI, "")

QmitkOtsuTool3DGUI::QmitkOtsuTool3DGUI()
:QmitkToolGUI(),
 m_Spinbox(NULL)
{
  // create the visible widgets
  QBoxLayout* mainLayout = new QHBoxLayout( this );

  QLabel* label = new QLabel( "Select number of Regions of Interst: ", this );
  QFont f = label->font();
  f.setBold(false);
  label->setFont( f );
  mainLayout->addWidget(label);

  m_Spinbox = new QSpinBox( this );
  m_Spinbox->setRange(2, 32);
  m_Spinbox->setValue(2);
  mainLayout->addWidget( m_Spinbox );

  QPushButton* okButton = new QPushButton("Create Segmentation", this);
  connect( okButton, SIGNAL(clicked()), this, SLOT(OnSpinboxValueAccept()));
  okButton->setFont( f );
  mainLayout->addWidget( okButton );

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkOtsuTool3DGUI::~QmitkOtsuTool3DGUI()
{

}

void QmitkOtsuTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  m_OtsuTool3DTool = dynamic_cast<mitk::OtsuTool3D*>( tool );
}

void QmitkOtsuTool3DGUI::OnSpinboxValueAccept()
{
  if (m_OtsuTool3DTool.IsNotNull())
  {
    m_OtsuTool3DTool->RunSegmentation( m_Spinbox->value() );
  }
}


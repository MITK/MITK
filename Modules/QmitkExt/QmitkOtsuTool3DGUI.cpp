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
#include <qlistwidget.h>

MITK_TOOL_GUI_MACRO(QmitkExt_EXPORT, QmitkOtsuTool3DGUI, "")

QmitkOtsuTool3DGUI::QmitkOtsuTool3DGUI()
:QmitkToolGUI(),
 m_Spinbox(NULL),
 m_selectionListWidget(NULL)
{
  // create the visible widgets
  QGridLayout* mainLayout = new QGridLayout( this );

  QLabel* label = new QLabel( "Select number of Regions of Interest: ", this );
  QFont f = label->font();
  f.setBold(false);
  label->setFont( f );
  mainLayout->addWidget(label,0,0);

  m_Spinbox = new QSpinBox( this );
  m_Spinbox->setRange(2, 32);
  m_Spinbox->setValue(2);
  mainLayout->addWidget( m_Spinbox,0,1 );

  //Button for confirming the selected number of regions for input of otsu algorithm
  QPushButton* okButton = new QPushButton("Accept", this);
  connect( okButton, SIGNAL(clicked()), this, SLOT(OnSpinboxValueAccept()));
  okButton->setFont( f );
  mainLayout->addWidget( okButton,0,3 );

  //QListWidget for User selection of desired result region
  m_selectionListWidget = new QListWidget(this);
  connect( m_selectionListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(OnItemSelectionChanged()));
  mainLayout->addWidget( m_selectionListWidget,1,0 );

  //Button for confirming the selected region of the QListWidget as segmentation result
  QPushButton* confirmButton = new QPushButton("Confirm Segmentation", this);
  connect( confirmButton, SIGNAL(clicked()), this, SLOT(OnSegmentationRegionAccept()));
  confirmButton->setFont( f );
  mainLayout->addWidget( confirmButton,1,1 );

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkOtsuTool3DGUI::~QmitkOtsuTool3DGUI()
{

}

void QmitkOtsuTool3DGUI::OnItemSelectionChanged()
{
  if (m_OtsuTool3DTool.IsNotNull())
  {
    // TODO update preview of region
    m_OtsuTool3DTool->UpdateBinaryPreview(m_selectionListWidget->currentItem()->text().toInt());
  }
}

void QmitkOtsuTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  m_OtsuTool3DTool = dynamic_cast<mitk::OtsuTool3D*>( tool );
}

void QmitkOtsuTool3DGUI::OnSegmentationRegionAccept()
{
  if (m_OtsuTool3DTool.IsNotNull() && this->m_selectionListWidget->currentItem() != NULL)
  {
    m_OtsuTool3DTool->ConfirmSegmentation();
    // TODO deactivate otsu
  }
}

void QmitkOtsuTool3DGUI::OnSpinboxValueAccept()
{
  if (m_OtsuTool3DTool.IsNotNull())
  {
    try
    {
      m_OtsuTool3DTool->RunSegmentation( m_Spinbox->value() );
    }
    catch( ... )
    {
      return;
    }
    //insert regions into widget
    QString itemName;
    QListWidgetItem* item;
    this->m_selectionListWidget->clear();
    for(int i=0; i<m_Spinbox->value(); ++i) {
      itemName = QString::number(i);
      item = new QListWidgetItem(itemName);
      this->m_selectionListWidget->addItem(item);
    }
  }
}

void QmitkOtsuTool3DGUI::OnVolumePreviewChecked(int state)
{
  if (state == 1)
  {
  }
}

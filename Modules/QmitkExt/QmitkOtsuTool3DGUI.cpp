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
:QmitkToolGUI()
{
  m_Controls.setupUi(this);

  connect( m_Controls.okButton, SIGNAL(clicked()), this, SLOT(OnSpinboxValueAccept()));
  connect( m_Controls.m_selectionListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(OnItemSelectionChanged()));
  connect( m_Controls.m_ConfSegButton, SIGNAL(clicked()), this, SLOT(OnSegmentationRegionAccept()));
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
    m_OtsuTool3DTool->UpdateBinaryPreview(m_Controls.m_selectionListWidget->currentItem()->text().toInt());
    if ( !m_Controls.m_selectionListWidget->selectedItems().empty() )
    {
      m_Controls.m_ConfSegButton->setEnabled( true );
    }
    else
    {
      m_Controls.m_ConfSegButton->setEnabled( false );
    }
  }
}

void QmitkOtsuTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  m_OtsuTool3DTool = dynamic_cast<mitk::OtsuTool3D*>( tool );
}

void QmitkOtsuTool3DGUI::OnSegmentationRegionAccept()
{
  if (m_OtsuTool3DTool.IsNotNull() && m_Controls.m_selectionListWidget->currentItem() != NULL)
  {
    m_OtsuTool3DTool->ConfirmSegmentation();
    m_OtsuTool3DTool->Deactivated();
  }
}

void QmitkOtsuTool3DGUI::OnSpinboxValueAccept()
{
  if (m_OtsuTool3DTool.IsNotNull())
  {
    try
    {
      this->setCursor(Qt::WaitCursor);
      m_OtsuTool3DTool->RunSegmentation( m_Controls.m_Spinbox->value() );
      this->setCursor(Qt::ArrowCursor);
    }
    catch( ... )
    {
      this->setCursor(Qt::ArrowCursor);
      return;
    }
    //insert regions into widget
    QString itemName;
    QListWidgetItem* item;
    m_Controls.m_selectionListWidget->clear();
    for(int i=0; i<m_Controls.m_Spinbox->value(); ++i) {
      itemName = QString::number(i);
      item = new QListWidgetItem(itemName);
      m_Controls.m_selectionListWidget->addItem(item);
    }
  }
}

void QmitkOtsuTool3DGUI::OnVolumePreviewChecked(int state)
{
  if (state == 1)
  {
  }
}

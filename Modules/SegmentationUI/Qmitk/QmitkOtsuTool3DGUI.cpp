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
#include "QmitkConfirmSegmentationDialog.h"

#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlistwidget.h>
#include <QMessageBox>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkOtsuTool3DGUI, "")

QmitkOtsuTool3DGUI::QmitkOtsuTool3DGUI()
:QmitkToolGUI(),
m_NumberOfRegions(0)
{
  m_Controls.setupUi(this);

  connect( m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnSpinboxValueAccept()));
  connect(m_Controls.m_selectionListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
          this, SLOT(OnItemSelectionChanged(QListWidgetItem*)));
  connect( m_Controls.m_ConfSegButton, SIGNAL(clicked()), this, SLOT(OnSegmentationRegionAccept()));
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkOtsuTool3DGUI::~QmitkOtsuTool3DGUI()
{

}

void QmitkOtsuTool3DGUI::OnItemSelectionChanged(QListWidgetItem* item)
{
  if (m_SelectedItem == item)
  {
    m_SelectedItem = 0;
    m_Controls.m_selectionListWidget->clearSelection();
    m_Controls.m_ConfSegButton->setEnabled( false );
    m_OtsuTool3DTool->ShowMultiLabelResultNode(true);
    return;
  }

  m_SelectedItem = item;

  if (m_OtsuTool3DTool.IsNotNull())
  {
    // TODO update preview of region
    m_OtsuTool3DTool->UpdateBinaryPreview(m_SelectedItem->text().toInt());
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
  QmitkConfirmSegmentationDialog dialog;
  QString segName = QString::fromStdString(m_OtsuTool3DTool->GetCurrentSegmentationName());

  dialog.SetSegmentationName(segName);
  int result = dialog.exec();

  switch(result)
  {
  case QmitkConfirmSegmentationDialog::CREATE_NEW_SEGMENTATION:
    m_OtsuTool3DTool->SetOverwriteExistingSegmentation(false);
    break;
  case QmitkConfirmSegmentationDialog::OVERWRITE_SEGMENTATION:
    m_OtsuTool3DTool->SetOverwriteExistingSegmentation(true);
    break;
  case QmitkConfirmSegmentationDialog::CANCEL_SEGMENTATION:
    return;
  }

  if (m_OtsuTool3DTool.IsNotNull() && m_Controls.m_selectionListWidget->currentItem() != NULL)
  {
    m_OtsuTool3DTool->ConfirmSegmentation();
  }
}

void QmitkOtsuTool3DGUI::OnSpinboxValueAccept()
{
  if( m_NumberOfRegions == m_Controls.m_Spinbox->value() )
    return;

  if (m_OtsuTool3DTool.IsNotNull())
  {
    try
    {
      m_NumberOfRegions = m_Controls.m_Spinbox->value();
      int proceed;

      QMessageBox* messageBox = new QMessageBox(QMessageBox::Question, NULL, "The otsu segmentation computation may take several minutes depending on the number of Regions you selected. Proceed anyway?", QMessageBox::Ok | QMessageBox::Cancel);
      if (m_NumberOfRegions >= 5)
      {
        proceed = messageBox->exec();
        if (proceed != QMessageBox::Ok) return;
      }
      this->setCursor(Qt::WaitCursor);
      m_OtsuTool3DTool->RunSegmentation( m_NumberOfRegions );
      this->setCursor(Qt::ArrowCursor);
    }
    catch( ... )
    {
      this->setCursor(Qt::ArrowCursor);
      QMessageBox* messageBox = new QMessageBox(QMessageBox::Critical, NULL, "itkOtsuFilter error: image dimension must be in {2, 3} and no RGB images can be handled.");
      messageBox->exec();
      delete messageBox;
      return;
    }
    //insert regions into widget
    QString itemName;
    QListWidgetItem* item;
    m_Controls.m_selectionListWidget->clear();
    for(int i=0; i<m_Controls.m_Spinbox->value(); ++i)
    {
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

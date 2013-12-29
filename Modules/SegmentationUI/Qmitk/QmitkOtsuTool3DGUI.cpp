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

#include <QmitkNewSegmentationDialog.h>
#include <QApplication.h>

#include <QMessageBox>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkOtsuTool3DGUI, "")

QmitkOtsuTool3DGUI::QmitkOtsuTool3DGUI() : QmitkToolGUI(), m_NumberOfRegions(0), m_OtsuTool3D(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_sbNumberOfRegions, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfRegionsChanged(int)) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );

  //connect( m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnSpinboxValueAccept()));
  connect(m_Controls.m_SelectionListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
          this, SLOT(OnItemSelectionChanged(QListWidgetItem*)));
//  connect( m_Controls.m_ConfSegButton, SIGNAL(clicked()), this, SLOT(OnSegmentationRegionAccept()));
}

QmitkOtsuTool3DGUI::~QmitkOtsuTool3DGUI()
{

}

void QmitkOtsuTool3DGUI::OnItemSelectionChanged(QListWidgetItem* item)
{
  /*
  if (m_SelectedItem == item)
  {
    m_SelectedItem = 0;
    m_Controls.m_SelectionListWidget->clearSelection();
    m_OtsuTool3DTool->ShowMultiLabelResultNode(true);
    return;
  }

  m_SelectedItem = item;

  if (m_OtsuTool3DTool.IsNotNull())
  {
    // TODO update preview of region
    m_OtsuTool3DTool->UpdateBinaryPreview(m_SelectedItem->text().toInt());
    if ( !m_Controls.m_SelectionListWidget->selectedItems().empty() )
    {
      m_Controls.m_ConfSegButton->setEnabled( true );
    }
    else
    {
      m_Controls.m_ConfSegButton->setEnabled( false );
    }
  }
  */
}

void QmitkOtsuTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_OtsuTool3D)
  {
    m_OtsuTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkOtsuTool3DGUI, bool>( this, &QmitkOtsuTool3DGUI::BusyStateChanged );
  }

  m_OtsuTool3D = dynamic_cast<mitk::OtsuTool3D*>( tool );

  if (m_OtsuTool3D)
  {
    m_OtsuTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkOtsuTool3DGUI, bool>( this, &QmitkOtsuTool3DGUI::BusyStateChanged );
  }
}
/*
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
    m_OtsuTool3DTool->Deactivated();
  }
}
*/

void QmitkOtsuTool3DGUI::OnRun()
{
  if( m_NumberOfRegions == m_Controls.m_sbNumberOfRegions->value() )
    return;

  if (m_OtsuTool3D.IsNotNull())
  {
    try
    {
      m_NumberOfRegions = m_Controls.m_sbNumberOfRegions->value();
      int proceed(false);
      if (m_NumberOfRegions >= 5)
      {
        proceed = QMessageBox::question(this, "Otsu tool",
          tr("The otsu segmentation computation may take several minutes depending on the number of Regions you selected. Proceed anyway?"), QMessageBox::Ok, QMessageBox::Cancel);
        if (proceed != QMessageBox::Ok) return;
      }
      m_OtsuTool3D->Run();
    }
    catch( ... )
    {
      QMessageBox::critical(this, "Otsu tool", "itkOtsuFilter error: image dimension must be in {2, 3} and no RGB images can be handled.");
      return;
    }
    //insert regions into widget
    QString itemName;
    QListWidgetItem* item;
    m_Controls.m_SelectionListWidget->clear();
    for(int i=0; i<m_Controls.m_sbNumberOfRegions->value(); ++i)
    {
      itemName = QString::number(i);
      item = new QListWidgetItem(itemName);
      m_Controls.m_SelectionListWidget->addItem(item);
    }
  }
}

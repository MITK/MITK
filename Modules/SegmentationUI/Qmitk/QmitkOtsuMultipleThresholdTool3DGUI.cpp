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

#include "QmitkOtsuMultipleThresholdTool3DGUI.h"

#include "mitkOtsuMultipleThresholdTool3D.h"

#include <QmitkNewSegmentationDialog.h>

#include <QApplication.h>
#include <QMessageBox>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkOtsuMultipleThresholdTool3DGUI, "")

QmitkOtsuMultipleThresholdTool3DGUI::QmitkOtsuMultipleThresholdTool3DGUI() : QmitkToolGUI(), m_NumberOfRegions(0), m_OtsuMultipleThresholdTool3D(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();


  m_Controls.m_SelectionListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
//  connect( m_Controls.m_sbNumberOfRegions, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfRegionsChanged(int)) );
  connect( m_Controls.m_pbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
  connect(m_Controls.m_SelectionListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
          this, SLOT(OnItemSelectionChanged(QListWidgetItem*)));
//  connect( m_Controls.m_ConfSegButton, SIGNAL(clicked()), this, SLOT(OnSegmentationRegionAccept()));
}

QmitkOtsuMultipleThresholdTool3DGUI::~QmitkOtsuMultipleThresholdTool3DGUI()
{
  if (m_OtsuMultipleThresholdTool3D)
  {
    m_OtsuMultipleThresholdTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkOtsuMultipleThresholdTool3DGUI, bool>( this, &QmitkOtsuMultipleThresholdTool3DGUI::BusyStateChanged );
  }
}

void QmitkOtsuMultipleThresholdTool3DGUI::OnItemSelectionChanged(QListWidgetItem* item)
{
  if (m_SelectedItem == item)
  {
    m_SelectedItem = 0;
    m_Controls.m_SelectionListWidget->clearSelection();
    m_OtsuMultipleThresholdTool3D->UpdatePreview(-1);
    return;
  }

  m_SelectedItem = item;

  if (m_OtsuMultipleThresholdTool3D)
  {
    m_OtsuMultipleThresholdTool3D->UpdatePreview(m_Controls.m_SelectionListWidget->currentRow());
  }
}

void QmitkOtsuMultipleThresholdTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_OtsuMultipleThresholdTool3D)
  {
    m_OtsuMultipleThresholdTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkOtsuMultipleThresholdTool3DGUI, bool>( this, &QmitkOtsuMultipleThresholdTool3DGUI::BusyStateChanged );
  }

  m_OtsuMultipleThresholdTool3D = dynamic_cast<mitk::OtsuMultipleThresholdTool3D*>( tool );

  if (m_OtsuMultipleThresholdTool3D)
  {
    m_Controls.m_sbNumberOfRegions->setValue(m_OtsuMultipleThresholdTool3D->GetNumberOfRegions());

    m_OtsuMultipleThresholdTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkOtsuMultipleThresholdTool3DGUI, bool>( this, &QmitkOtsuMultipleThresholdTool3DGUI::BusyStateChanged );
  }
}

void QmitkOtsuMultipleThresholdTool3DGUI::OnAcceptPreview()
{
  if (m_OtsuMultipleThresholdTool3D)
  {
//    m_OtsuMultipleThresholdTool3D->AcceptPreview(m_Controls.m_SelectionListWidget->currentRow());
    QmitkNewSegmentationDialog dialog(this);
//    dialog->SetSuggestionList( m_OrganColors );
    dialog.setWindowTitle("New Label");
    int dialogReturnValue = dialog.exec();
    if ( dialogReturnValue == QDialog::Rejected ) return;
    mitk::Color color = dialog.GetColor();
    std::string name = dialog.GetSegmentationName().toStdString();
    m_OtsuMultipleThresholdTool3D->CreateNewLabel(name, color);
  }
}

void QmitkOtsuMultipleThresholdTool3DGUI::OnRun()
{
  if( m_NumberOfRegions == m_Controls.m_sbNumberOfRegions->value() )
  {
    QMessageBox::information(this, "Otsu tool",
        tr("You have requested to extract the same number of regions as in the last run.\nResults are already available."), QMessageBox::Ok);
    return;
  }

  if (m_OtsuMultipleThresholdTool3D)
  {
    m_NumberOfRegions = m_Controls.m_sbNumberOfRegions->value();
    int proceed(false);
    if (m_NumberOfRegions >= 5)
    {
      proceed = QMessageBox::question(this, "Otsu tool",
        tr("Extractig more than 4 regions may take several minutes. Proceed anyway?"), QMessageBox::Ok, QMessageBox::Cancel);
      if (proceed != QMessageBox::Ok) return;
    }

    m_OtsuMultipleThresholdTool3D->SetNumberOfRegions(m_Controls.m_sbNumberOfRegions->value());
    m_OtsuMultipleThresholdTool3D->Run();

    //insert regions into widget
    QListWidgetItem* item;
    m_Controls.m_SelectionListWidget->clear();
    for (int i=0; i<m_Controls.m_sbNumberOfRegions->value(); ++i)
    {
      QString itemName = "region-" + QString::number(i);
      item = new QListWidgetItem(itemName);
      m_Controls.m_SelectionListWidget->addItem(item);
    }

    m_Controls.m_SelectionListWidget->setCurrentRow(0);
    this->OnItemSelectionChanged(m_Controls.m_SelectionListWidget->currentItem());
  }
}

void QmitkOtsuMultipleThresholdTool3DGUI::OnCancel()
{
  if (m_OtsuMultipleThresholdTool3D)
  {
    m_OtsuMultipleThresholdTool3D->Cancel();
  }
}

void QmitkOtsuMultipleThresholdTool3DGUI::OnShowInformation( bool value )
{
  if (value)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkOtsuMultipleThresholdTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

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

  m_Controls.m_SelectionListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
//  connect( m_Controls.m_sbNumberOfRegions, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfRegionsChanged(int)) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
  connect(m_Controls.m_SelectionListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
          this, SLOT(OnItemSelectionChanged(QListWidgetItem*)));
//  connect( m_Controls.m_ConfSegButton, SIGNAL(clicked()), this, SLOT(OnSegmentationRegionAccept()));
}

QmitkOtsuTool3DGUI::~QmitkOtsuTool3DGUI()
{
  if (m_OtsuTool3D)
  {
    m_OtsuTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkOtsuTool3DGUI, bool>( this, &QmitkOtsuTool3DGUI::BusyStateChanged );
  }
}

void QmitkOtsuTool3DGUI::OnItemSelectionChanged(QListWidgetItem* item)
{
  if (m_SelectedItem == item)
  {
    m_SelectedItem = 0;
    m_Controls.m_SelectionListWidget->clearSelection();
    m_OtsuTool3D->UpdatePreview(-1);
    return;
  }

  m_SelectedItem = item;

  if (m_OtsuTool3D)
  {
    m_OtsuTool3D->UpdatePreview(m_Controls.m_SelectionListWidget->currentRow());
  }
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
    m_Controls.m_sbNumberOfRegions->setValue(m_OtsuTool3D->GetNumberOfRegions());

    m_OtsuTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkOtsuTool3DGUI, bool>( this, &QmitkOtsuTool3DGUI::BusyStateChanged );
  }
}

void QmitkOtsuTool3DGUI::OnAcceptPreview()
{
  if (m_OtsuTool3D)
  {
//    m_OtsuTool3D->AcceptPreview(m_Controls.m_SelectionListWidget->currentRow());
    QmitkNewSegmentationDialog dialog(this);
//    dialog->SetSuggestionList( m_OrganColors );
    dialog.setWindowTitle("New Label");
    int dialogReturnValue = dialog.exec();
    if ( dialogReturnValue == QDialog::Rejected ) return;
    mitk::Color color = dialog.GetColor();
    std::string name = dialog.GetSegmentationName().toStdString();
    m_OtsuTool3D->CreateNewLabel(name, color);
  }
}

void QmitkOtsuTool3DGUI::OnRun()
{
  if( m_NumberOfRegions == m_Controls.m_sbNumberOfRegions->value() )
  {
    QMessageBox::information(this, "Otsu tool",
        tr("You have requested to extract the same number of regions as in the last run.\nResults are already available."), QMessageBox::Ok);
    return;
  }

  if (m_OtsuTool3D)
  {
    m_NumberOfRegions = m_Controls.m_sbNumberOfRegions->value();
    int proceed(false);
    if (m_NumberOfRegions >= 5)
    {
      proceed = QMessageBox::question(this, "Otsu tool",
        tr("Extractig more than 4 regions may take several minutes. Proceed anyway?"), QMessageBox::Ok, QMessageBox::Cancel);
      if (proceed != QMessageBox::Ok) return;
    }

    m_OtsuTool3D->SetNumberOfRegions(m_Controls.m_sbNumberOfRegions->value());
    m_OtsuTool3D->Run();

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

void QmitkOtsuTool3DGUI::OnCancel()
{
  if (m_OtsuTool3D)
  {
    m_OtsuTool3D->Cancel();
  }
}

void QmitkOtsuTool3DGUI::OnShowInformation( bool value )
{
  if (value)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkOtsuTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

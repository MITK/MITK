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

#include "mitkOtsuTool3D.h"

#include <QmitkNewSegmentationDialog.h>

#include <QApplication.h>
#include <QListWidgetItem>
#include <QMessageBox>

MITK_TOOL_GUI_MACRO(MitkSegmentationUI_EXPORT, QmitkOtsuTool3DGUI, "")

QmitkOtsuTool3DGUI::QmitkOtsuTool3DGUI() : QmitkToolGUI(), m_NumberOfRegions(0), m_OtsuMultipleThresholdTool3D(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();

  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect(m_Controls.m_selectionListWidget, SIGNAL(itemSelectionChanged()),
          this, SLOT(OnItemSelectionChanged()));
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
//  connect( m_Controls.m_ConfSegButton, SIGNAL(clicked()), this, SLOT(OnSegmentationRegionAccept()));
  connect(m_Controls.advancedSettingsButton, SIGNAL(toggled(bool)), this, SLOT(OnAdvancedSettingsButtonToggled(bool)));
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
  connect( m_Controls.m_pbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  this->OnAdvancedSettingsButtonToggled(false);
}

QmitkOtsuTool3DGUI::~QmitkOtsuTool3DGUI()
{
  if (m_OtsuMultipleThresholdTool3D)
  {
    m_OtsuMultipleThresholdTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkOtsuTool3DGUI, bool>( this, &QmitkOtsuTool3DGUI::BusyStateChanged );
  }
}

void QmitkOtsuTool3DGUI::OnItemSelectionChanged()
{
  m_SelectedItems = m_Controls.m_selectionListWidget->selectedItems();

  if (m_SelectedItems.size() == 0)
  {
    m_Controls.m_pbAcceptPreview->setEnabled( false );
//    m_OtsuTool3DTool->ShowMultiLabelResultNode(true);
    return;
  }

  if (m_OtsuMultipleThresholdTool3D)
  {
    // update preview of region
    QList<QListWidgetItem *>::Iterator it;
    std::vector<int> regionIDs;
    for (it = m_SelectedItems.begin(); it != m_SelectedItems.end(); ++it)
      regionIDs.push_back((*it)->text().toInt());
    m_OtsuMultipleThresholdTool3D->UpdatePreview(regionIDs);
    m_Controls.m_pbAcceptPreview->setEnabled( true );
  }
}

void QmitkOtsuTool3DGUI::OnAdvancedSettingsButtonToggled(bool toggled)
{
  m_Controls.m_ValleyCheckbox->setVisible(toggled);
  m_Controls.binLabel->setVisible(toggled);
  m_Controls.m_BinsSpinBox->setVisible(toggled);
}

void QmitkOtsuTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_OtsuMultipleThresholdTool3D)
  {
    m_OtsuMultipleThresholdTool3D->CurrentlyBusy -= mitk::MessageDelegate1<QmitkOtsuTool3DGUI, bool>( this, &QmitkOtsuTool3DGUI::BusyStateChanged );
  }

  m_OtsuMultipleThresholdTool3D = dynamic_cast<mitk::OtsuMultipleThresholdTool3D*>( tool );

  if (m_OtsuMultipleThresholdTool3D)
  {
    m_Controls.m_sbNumberOfRegions->setValue(m_OtsuMultipleThresholdTool3D->GetNumberOfRegions());

    m_OtsuMultipleThresholdTool3D->CurrentlyBusy += mitk::MessageDelegate1<QmitkOtsuTool3DGUI, bool>( this, &QmitkOtsuTool3DGUI::BusyStateChanged );
  }
}

void QmitkOtsuTool3DGUI::OnAcceptPreview()
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

void QmitkOtsuTool3DGUI::OnRun()
{
  if( m_NumberOfRegions == m_Controls.m_sbNumberOfRegions->value() &&
      m_UseValleyEmphasis == m_Controls.m_ValleyCheckbox->isChecked() &&
      m_NumberOfBins == m_Controls.m_BinsSpinBox->value() )
    return;

  if (m_OtsuMultipleThresholdTool3D)
  {
    try
    {
      m_NumberOfRegions = m_Controls.m_sbNumberOfRegions->value();
      m_UseValleyEmphasis = m_Controls.m_ValleyCheckbox->isChecked();
      m_NumberOfBins = m_Controls.m_BinsSpinBox->value();
      int proceed;

      QMessageBox* messageBox = new QMessageBox(QMessageBox::Question, NULL,
                                                "The otsu segmentation computation may take several minutes depending on the number of Regions you selected.Proceed anyway?", QMessageBox::Ok | QMessageBox::Cancel);
      if (m_NumberOfRegions >= 5)
      {
        proceed = messageBox->exec();
        if (proceed != QMessageBox::Ok) return;
      }
      this->setCursor(Qt::WaitCursor);
      m_OtsuMultipleThresholdTool3D->Run( m_NumberOfRegions, m_UseValleyEmphasis, m_NumberOfBins );
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
    QListWidgetItem* item;
    m_Controls.m_SelectionListWidget->clear();
    for (int i=0; i<m_Controls.m_sbNumberOfRegions->value(); ++i)
    {
      QString itemName = "region-" + QString::number(i);
      item = new QListWidgetItem(itemName);
      m_Controls.m_SelectionListWidget->addItem(item);
    }

    m_Controls.m_pbAcceptPreview->setEnabled(false);
  }
}

void QmitkOtsuTool3DGUI::OnCancel()
{
  if (m_OtsuMultipleThresholdTool3D)
  {
    m_OtsuMultipleThresholdTool3D->Cancel();
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

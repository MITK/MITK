/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkOtsuTool3DGUI.h"

#include <QMessageBox>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkOtsuTool3DGUI, "")

QmitkOtsuTool3DGUI::QmitkOtsuTool3DGUI() : QmitkToolGUI(), m_NumberOfRegions(0)
{
  m_Controls.setupUi(this);

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnSpinboxValueAccept()));
  connect(m_Controls.m_selectionListWidget, &QmitkSimpleLabelSetListWidget::SelectedLabelsChanged, this, &QmitkOtsuTool3DGUI::OnRegionSelectionChanged);
  connect(m_Controls.m_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(OnRegionSpinboxChanged(int)));
  connect(m_Controls.m_ConfSegButton, SIGNAL(clicked()), this, SLOT(OnSegmentationRegionAccept()));
  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
  connect(m_Controls.advancedSettingsButton, SIGNAL(toggled(bool)), this, SLOT(OnAdvancedSettingsButtonToggled(bool)));

  this->OnAdvancedSettingsButtonToggled(false);
}

QmitkOtsuTool3DGUI::~QmitkOtsuTool3DGUI()
{
  if (m_OtsuTool3DTool.IsNotNull())
  {
    m_OtsuTool3DTool->CurrentlyBusy -=
      mitk::MessageDelegate1<QmitkOtsuTool3DGUI, bool>(this, &QmitkOtsuTool3DGUI::BusyStateChanged);
  }
}

void QmitkOtsuTool3DGUI::OnRegionSpinboxChanged(int numberOfRegions)
{
  // we have to change to minimum number of histogram bins accordingly
  int curBinValue = m_Controls.m_BinsSpinBox->value();
  if (curBinValue < numberOfRegions)
    m_Controls.m_BinsSpinBox->setValue(numberOfRegions);
}

void QmitkOtsuTool3DGUI::OnRegionSelectionChanged(const QmitkSimpleLabelSetListWidget::LabelVectorType& selectedLabels)
{
  if (m_OtsuTool3DTool.IsNotNull())
  {
    mitk::AutoMLSegmentationWithPreviewTool::SelectedLabelVectorType labelIDs;
    for (const auto& label : selectedLabels)
    {
      labelIDs.push_back(label->GetValue());
    }

    m_OtsuTool3DTool->SetSelectedLabels(labelIDs);
    m_OtsuTool3DTool->UpdatePreview();

    m_Controls.m_ConfSegButton->setEnabled(!labelIDs.empty());
  }
}

void QmitkOtsuTool3DGUI::OnAdvancedSettingsButtonToggled(bool toggled)
{
  m_Controls.m_ValleyCheckbox->setVisible(toggled);
  m_Controls.binLabel->setVisible(toggled);
  m_Controls.m_BinsSpinBox->setVisible(toggled);

  if (toggled)
  {
    int max = m_OtsuTool3DTool->GetMaxNumberOfBins();
    if (max >= m_Controls.m_BinsSpinBox->minimum())
    {
      m_Controls.m_BinsSpinBox->setMaximum(max);
    }
  }
}

void QmitkOtsuTool3DGUI::OnNewToolAssociated(mitk::Tool *tool)
{
  if (m_OtsuTool3DTool.IsNotNull())
  {
    m_OtsuTool3DTool->CurrentlyBusy -=
      mitk::MessageDelegate1<QmitkOtsuTool3DGUI, bool>(this, &QmitkOtsuTool3DGUI::BusyStateChanged);
  }

  m_OtsuTool3DTool = dynamic_cast<mitk::OtsuTool3D *>(tool);

  if (m_OtsuTool3DTool.IsNotNull())
  {
    m_OtsuTool3DTool->CurrentlyBusy +=
      mitk::MessageDelegate1<QmitkOtsuTool3DGUI, bool>(this, &QmitkOtsuTool3DGUI::BusyStateChanged);

    m_OtsuTool3DTool->SetOverwriteExistingSegmentation(true);
    m_OtsuTool3DTool->IsTimePointChangeAwareOff();
    m_Controls.m_CheckProcessAll->setVisible(m_OtsuTool3DTool->GetTargetSegmentationNode()->GetData()->GetTimeSteps() > 1);
  }
}

void QmitkOtsuTool3DGUI::OnSegmentationRegionAccept()
{
  QString segName = QString::fromStdString(m_OtsuTool3DTool->GetCurrentSegmentationName());

  if (m_OtsuTool3DTool.IsNotNull())
  {
    if (this->m_Controls.m_CheckCreateNew->isChecked())
    {
      m_OtsuTool3DTool->SetOverwriteExistingSegmentation(false);
    }
    else
    {
      m_OtsuTool3DTool->SetOverwriteExistingSegmentation(true);
    }

    m_OtsuTool3DTool->SetCreateAllTimeSteps(this->m_Controls.m_CheckProcessAll->isChecked());

    this->m_Controls.m_ConfSegButton->setEnabled(false);
    m_OtsuTool3DTool->ConfirmSegmentation();
  }
}

void QmitkOtsuTool3DGUI::OnSpinboxValueAccept()
{
  if (m_NumberOfRegions == m_Controls.m_Spinbox->value() &&
      m_UseValleyEmphasis == m_Controls.m_ValleyCheckbox->isChecked() &&
      m_NumberOfBins == m_Controls.m_BinsSpinBox->value())
    return;

  if (m_OtsuTool3DTool.IsNotNull())
  {
    try
    {
      int proceed;
      QMessageBox *messageBox = new QMessageBox(QMessageBox::Question,
                                                nullptr,
                                                "The otsu segmentation computation may take several minutes depending "
                                                "on the number of Regions you selected. Proceed anyway?",
                                                QMessageBox::Ok | QMessageBox::Cancel);
      if (m_Controls.m_Spinbox->value() >= 5)
      {
        proceed = messageBox->exec();
        if (proceed != QMessageBox::Ok)
          return;
      }

      m_NumberOfRegions = m_Controls.m_Spinbox->value();
      m_UseValleyEmphasis = m_Controls.m_ValleyCheckbox->isChecked();
      m_NumberOfBins = m_Controls.m_BinsSpinBox->value();
      m_OtsuTool3DTool->SetNumberOfRegions(m_NumberOfRegions);
      m_OtsuTool3DTool->SetUseValley(m_UseValleyEmphasis);
      m_OtsuTool3DTool->SetNumberOfBins(m_NumberOfBins);

      m_OtsuTool3DTool->UpdatePreview();
    }
    catch (...)
    {
      this->setCursor(Qt::ArrowCursor);
      QMessageBox *messageBox =
        new QMessageBox(QMessageBox::Critical,
                        nullptr,
                        "itkOtsuFilter error: image dimension must be in {2, 3} and no RGB images can be handled.");
      messageBox->exec();
      delete messageBox;
      return;
    }

    m_Controls.m_selectionListWidget->SetLabelSetImage(m_OtsuTool3DTool->GetMLPreview());
    m_OtsuTool3DTool->IsTimePointChangeAwareOn();
  }
}

void QmitkOtsuTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
  {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }

  m_Controls.m_ValleyCheckbox->setEnabled(!value);
  m_Controls.binLabel->setEnabled(!value);
  m_Controls.m_BinsSpinBox->setEnabled(!value);
  m_Controls.m_ConfSegButton->setEnabled(!m_OtsuTool3DTool->GetSelectedLabels().empty() && !value);
  m_Controls.m_CheckProcessAll->setEnabled(!value);
  m_Controls.m_CheckCreateNew->setEnabled(!value);
  m_Controls.previewButton->setEnabled(!value);
}

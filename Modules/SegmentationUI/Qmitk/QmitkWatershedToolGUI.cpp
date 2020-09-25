/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkWatershedToolGUI.h"

#include <QMessageBox>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkWatershedToolGUI, "")

QmitkWatershedToolGUI::QmitkWatershedToolGUI() : QmitkToolGUI()
{
  m_Controls.setupUi(this);

  m_Controls.thresholdSlider->setMinimum(0);
  //We set the threshold maximum to 0.5 to avoid crashes in the watershed filter
  //see T27703 for more details.
  m_Controls.thresholdSlider->setMaximum(0.5);
  m_Controls.thresholdSlider->setValue(m_Threshold);
  m_Controls.thresholdSlider->setPageStep(0.01);
  m_Controls.thresholdSlider->setSingleStep(0.001);
  m_Controls.thresholdSlider->setDecimals(4);

  m_Controls.levelSlider->setMinimum(0);
  m_Controls.levelSlider->setMaximum(1);
  m_Controls.levelSlider->setValue(m_Level);
  m_Controls.levelSlider->setPageStep(0.1);
  m_Controls.levelSlider->setSingleStep(0.01);

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnSettingsAccept()));
  connect(m_Controls.m_selectionListWidget, &QmitkSimpleLabelSetListWidget::SelectedLabelsChanged, this, &QmitkWatershedToolGUI::OnRegionSelectionChanged);
  connect(m_Controls.levelSlider, SIGNAL(valueChanged(double)), this, SLOT(OnLevelChanged(double)));
  connect(m_Controls.thresholdSlider, SIGNAL(valueChanged(double)), this, SLOT(OnThresholdChanged(double)));
  connect(m_Controls.m_ConfSegButton, SIGNAL(clicked()), this, SLOT(OnSegmentationRegionAccept()));
  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
}

QmitkWatershedToolGUI::~QmitkWatershedToolGUI()
{
  if (m_WatershedTool.IsNotNull())
  {
    m_WatershedTool->CurrentlyBusy -=
      mitk::MessageDelegate1<QmitkWatershedToolGUI, bool>(this, &QmitkWatershedToolGUI::BusyStateChanged);
  }
}

void QmitkWatershedToolGUI::OnRegionSelectionChanged(const QmitkSimpleLabelSetListWidget::LabelVectorType& selectedLabels)
{
  if (m_WatershedTool.IsNotNull())
  {
    mitk::AutoMLSegmentationWithPreviewTool::SelectedLabelVectorType labelIDs;
    for (const auto& label : selectedLabels)
    {
      labelIDs.push_back(label->GetValue());
    }

    m_WatershedTool->SetSelectedLabels(labelIDs);
    m_WatershedTool->UpdatePreview();

    m_Controls.m_ConfSegButton->setEnabled(!labelIDs.empty());
  }
}

void QmitkWatershedToolGUI::OnNewToolAssociated(mitk::Tool *tool)
{
  if (m_WatershedTool.IsNotNull())
  {
    m_WatershedTool->CurrentlyBusy -=
      mitk::MessageDelegate1<QmitkWatershedToolGUI, bool>(this, &QmitkWatershedToolGUI::BusyStateChanged);
  }

  m_WatershedTool = dynamic_cast<mitk::WatershedTool *>(tool);

  if (m_WatershedTool.IsNotNull())
  {
    m_WatershedTool->CurrentlyBusy +=
      mitk::MessageDelegate1<QmitkWatershedToolGUI, bool>(this, &QmitkWatershedToolGUI::BusyStateChanged);

    m_WatershedTool->SetLevel(m_Level);
    m_WatershedTool->SetThreshold(m_Threshold);

    m_WatershedTool->SetOverwriteExistingSegmentation(true);
    m_WatershedTool->IsTimePointChangeAwareOff();
    m_Controls.m_CheckProcessAll->setVisible(m_WatershedTool->GetTargetSegmentationNode()->GetData()->GetTimeSteps() > 1);
  }
}

void QmitkWatershedToolGUI::OnSegmentationRegionAccept()
{
  QString segName = QString::fromStdString(m_WatershedTool->GetCurrentSegmentationName());

  if (m_WatershedTool.IsNotNull())
  {
    if (this->m_Controls.m_CheckCreateNew->isChecked())
    {
      m_WatershedTool->SetOverwriteExistingSegmentation(false);
    }
    else
    {
      m_WatershedTool->SetOverwriteExistingSegmentation(true);
    }

    m_WatershedTool->SetCreateAllTimeSteps(this->m_Controls.m_CheckProcessAll->isChecked());

    this->m_Controls.m_ConfSegButton->setEnabled(false);
    m_WatershedTool->ConfirmSegmentation();
  }
}

void QmitkWatershedToolGUI::OnSettingsAccept()
{
  if (m_WatershedTool.IsNotNull())
  {
    try
    {
      m_Threshold = m_Controls.thresholdSlider->value();
      m_Level = m_Controls.levelSlider->value();
      m_WatershedTool->SetThreshold(m_Threshold);
      m_WatershedTool->SetLevel(m_Level);

      m_WatershedTool->UpdatePreview();
    }
    catch (const std::exception& e)
    {
      this->setCursor(Qt::ArrowCursor);
      std::stringstream stream;
      stream << "Error while generation watershed segmentation. Reason: " << e.what();

      QMessageBox* messageBox =
        new QMessageBox(QMessageBox::Critical,
          nullptr, stream.str().c_str());
      messageBox->exec();
      delete messageBox;
      MITK_ERROR << stream.str();
      return;
    }
    catch (...)
    {
      this->setCursor(Qt::ArrowCursor);
      std::stringstream stream;
      stream << "Unkown error occured while generation watershed segmentation.";

      QMessageBox* messageBox =
        new QMessageBox(QMessageBox::Critical,
          nullptr, stream.str().c_str());
      messageBox->exec();
      delete messageBox;
      MITK_ERROR << stream.str();
      return;
    }

    m_Controls.m_selectionListWidget->SetLabelSetImage(m_WatershedTool->GetMLPreview());
    m_WatershedTool->IsTimePointChangeAwareOn();
  }
}

void QmitkWatershedToolGUI::BusyStateChanged(bool value)
{
  if (value)
  {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }

  m_Controls.levelSlider->setEnabled(!value);
  m_Controls.thresholdSlider->setEnabled(!value);
  m_Controls.m_ConfSegButton->setEnabled(!m_WatershedTool->GetSelectedLabels().empty() && !value);
  m_Controls.m_CheckProcessAll->setEnabled(!value);
  m_Controls.m_CheckCreateNew->setEnabled(!value);
  m_Controls.previewButton->setEnabled(!value);
}


void QmitkWatershedToolGUI::OnLevelChanged(double value)
{
  if (m_WatershedTool.IsNotNull())
  {
    m_WatershedTool->SetLevel(value);
  }
}

void QmitkWatershedToolGUI::OnThresholdChanged(double value)
{
  if (m_WatershedTool.IsNotNull())
  {
    m_WatershedTool->SetThreshold(value);
  }
}

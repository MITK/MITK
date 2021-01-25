/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkWatershedToolGUI.h"
#include "mitkWatershedTool.h"

#include <QMessageBox>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkWatershedToolGUI, "")

QmitkWatershedToolGUI::QmitkWatershedToolGUI() : QmitkAutoMLSegmentationToolGUIBase()
{
}

void QmitkWatershedToolGUI::ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool)
{
  Superclass::ConnectNewTool(newTool);

  auto tool = dynamic_cast<mitk::WatershedTool*>(newTool);
  if (nullptr != tool)
  {
    tool->SetLevel(m_Level);
    tool->SetThreshold(m_Threshold);
  }

  newTool->IsTimePointChangeAwareOff();
}

void QmitkWatershedToolGUI::InitializeUI(QBoxLayout* mainLayout)
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
  connect(m_Controls.levelSlider, SIGNAL(valueChanged(double)), this, SLOT(OnLevelChanged(double)));
  connect(m_Controls.thresholdSlider, SIGNAL(valueChanged(double)), this, SLOT(OnThresholdChanged(double)));

  mainLayout->addLayout(m_Controls.verticalLayout);

  Superclass::InitializeUI(mainLayout);
}

void QmitkWatershedToolGUI::OnSettingsAccept()
{
  auto tool = this->GetConnectedToolAs<mitk::WatershedTool>();
  if (nullptr != tool)
  {
    try
    {
      m_Threshold = m_Controls.thresholdSlider->value();
      m_Level = m_Controls.levelSlider->value();
      tool->SetThreshold(m_Threshold);
      tool->SetLevel(m_Level);

      tool->UpdatePreview();
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

    this->SetLabelSetPreview(tool->GetMLPreview());
    tool->IsTimePointChangeAwareOn();
  }
}

void QmitkWatershedToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
  m_Controls.levelSlider->setEnabled(enabled);
  m_Controls.thresholdSlider->setEnabled(enabled);
  m_Controls.previewButton->setEnabled(enabled);
}


void QmitkWatershedToolGUI::OnLevelChanged(double value)
{
  auto tool = this->GetConnectedToolAs<mitk::WatershedTool>();
  if (nullptr != tool)
  {
    tool->SetLevel(value);
  }
}

void QmitkWatershedToolGUI::OnThresholdChanged(double value)
{
  auto tool = this->GetConnectedToolAs<mitk::WatershedTool>();
  if (nullptr != tool)
  {
    tool->SetThreshold(value);
  }
}

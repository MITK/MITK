/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkOtsuTool3DGUI.h"
#include "mitkOtsuTool3D.h"

#include <QMessageBox>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkOtsuTool3DGUI, "")

QmitkOtsuTool3DGUI::QmitkOtsuTool3DGUI() : QmitkAutoMLSegmentationToolGUIBase()
{
}

void QmitkOtsuTool3DGUI::ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool)
{
  Superclass::ConnectNewTool(newTool);

  newTool->IsTimePointChangeAwareOff();
}

void QmitkOtsuTool3DGUI::InitializeUI(QBoxLayout* mainLayout)
{
  m_Controls.setupUi(this);
  mainLayout->addLayout(m_Controls.verticalLayout);

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnSpinboxValueAccept()));
  connect(m_Controls.m_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(OnRegionSpinboxChanged(int)));
  connect(m_Controls.advancedSettingsButton, SIGNAL(toggled(bool)), this, SLOT(OnAdvancedSettingsButtonToggled(bool)));

  this->OnAdvancedSettingsButtonToggled(false);

  Superclass::InitializeUI(mainLayout);
}

void QmitkOtsuTool3DGUI::OnRegionSpinboxChanged(int numberOfRegions)
{
  // we have to change to minimum number of histogram bins accordingly
  int curBinValue = m_Controls.m_BinsSpinBox->value();
  if (curBinValue < numberOfRegions)
    m_Controls.m_BinsSpinBox->setValue(numberOfRegions);
}

void QmitkOtsuTool3DGUI::OnAdvancedSettingsButtonToggled(bool toggled)
{
  m_Controls.m_ValleyCheckbox->setVisible(toggled);
  m_Controls.binLabel->setVisible(toggled);
  m_Controls.m_BinsSpinBox->setVisible(toggled);

  auto tool = this->GetConnectedToolAs<mitk::OtsuTool3D>();
  if (toggled && nullptr != tool)
  {
    int max = tool->GetMaxNumberOfBins();
    if (max >= m_Controls.m_BinsSpinBox->minimum())
    {
      m_Controls.m_BinsSpinBox->setMaximum(max);
    }
  }
}

void QmitkOtsuTool3DGUI::OnSpinboxValueAccept()
{
  if (m_NumberOfRegions == m_Controls.m_Spinbox->value() &&
      m_UseValleyEmphasis == m_Controls.m_ValleyCheckbox->isChecked() &&
      m_NumberOfBins == m_Controls.m_BinsSpinBox->value())
    return;

  auto tool = this->GetConnectedToolAs<mitk::OtsuTool3D>();
  if (nullptr != tool)
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
      tool->SetNumberOfRegions(m_NumberOfRegions);
      tool->SetUseValley(m_UseValleyEmphasis);
      tool->SetNumberOfBins(m_NumberOfBins);

      tool->UpdatePreview();
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

    this->SetLabelSetPreview(tool->GetMLPreview());
    tool->IsTimePointChangeAwareOn();
  }
}

void QmitkOtsuTool3DGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
  m_Controls.m_ValleyCheckbox->setEnabled(enabled);
  m_Controls.binLabel->setEnabled(enabled);
  m_Controls.m_BinsSpinBox->setEnabled(enabled);
  m_Controls.previewButton->setEnabled(enabled);
}

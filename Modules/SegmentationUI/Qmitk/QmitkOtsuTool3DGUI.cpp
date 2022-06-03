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

QmitkOtsuTool3DGUI::QmitkOtsuTool3DGUI() : QmitkMultiLabelSegWithPreviewToolGUIBase(), m_SuperclassEnableConfirmSegBtnFnc(m_EnableConfirmSegBtnFnc)
{
  auto enableMLSelectedDelegate = [this](bool enabled)
  {
    if (this->m_FirstPreviewComputation)
    {
      return false;
    }
    else
    {
      return this->m_SuperclassEnableConfirmSegBtnFnc(enabled);
    }
  };

  m_EnableConfirmSegBtnFnc = enableMLSelectedDelegate;
}

void QmitkOtsuTool3DGUI::ConnectNewTool(mitk::SegWithPreviewTool* newTool)
{
  Superclass::ConnectNewTool(newTool);

  newTool->IsTimePointChangeAwareOff();
  m_FirstPreviewComputation = true;
}

void QmitkOtsuTool3DGUI::InitializeUI(QBoxLayout* mainLayout)
{
  m_Controls.setupUi(this);
  mainLayout->addLayout(m_Controls.verticalLayout);

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnPreviewBtnClicked()));
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

void QmitkOtsuTool3DGUI::OnPreviewBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::OtsuTool3D>();
  if (nullptr != tool)
  {
    if (!m_FirstPreviewComputation &&
      (tool->GetNumberOfRegions() == static_cast<unsigned int>(m_Controls.m_Spinbox->value()) &&
      tool->GetUseValley() == m_Controls.m_ValleyCheckbox->isChecked() &&
      tool->GetNumberOfBins() == static_cast<unsigned int>(m_Controls.m_BinsSpinBox->value())))
      return;

    m_FirstPreviewComputation = false;

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

      tool->SetNumberOfRegions(static_cast<unsigned int>(m_Controls.m_Spinbox->value()));
      tool->SetUseValley(m_Controls.m_ValleyCheckbox->isChecked());
      tool->SetNumberOfBins(static_cast<unsigned int>(m_Controls.m_BinsSpinBox->value()));

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

    this->SetLabelSetPreview(tool->GetPreviewSegmentation());
    tool->IsTimePointChangeAwareOn();
    this->ActualizePreviewLabelVisibility();
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

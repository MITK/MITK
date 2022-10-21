/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkGrowCutToolGUI.h"
#include <mitkGrowCutTool.h>
#include <mitkToolManagerProvider.h>

// Qt
#include <QBoxLayout>
#include <QLabel>
#include <QRect>
#include <QPushButton>
#include <QRadioButton>
#include <QMessageBox>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkGrowCutToolGUI, "")

QmitkGrowCutToolGUI::QmitkGrowCutToolGUI() : QmitkMultiLabelSegWithPreviewToolGUIBase()
{
  auto enableConfirmSegBtnFnc = [this](bool enabled) { return enabled && m_FirstPreviewComputation; };

  m_EnableConfirmSegBtnFnc = enableConfirmSegBtnFnc;
}

QmitkGrowCutToolGUI::~QmitkGrowCutToolGUI() {}

void QmitkGrowCutToolGUI::ConnectNewTool(mitk::SegWithPreviewTool *newTool)
{
  Superclass::ConnectNewTool(newTool);

  newTool->IsTimePointChangeAwareOff();
}

void QmitkGrowCutToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  m_Controls.setupUi(this);

  this->OnAdvancedSettingsButtonToggled(false);

  std::function<bool()> isPreviewAvailable;
  isPreviewAvailable = [this]()
  {
    auto tool = this->GetConnectedToolAs<mitk::GrowCutTool>();
    if (nullptr != tool)
    {
      return tool->SeedImageIsValid();
    }
    return false;
  };

  auto previewAvailable = isPreviewAvailable();
  m_Controls.m_previewButton->setEnabled(previewAvailable);
  m_Controls.m_warningLabel->setVisible(!previewAvailable);

  mainLayout->addLayout(m_Controls.verticalLayout);

  connect(m_Controls.m_previewButton, &QPushButton::clicked, this, &QmitkGrowCutToolGUI::OnPreviewBtnClicked);
  connect(m_Controls.m_advancedSettingsButton,
          &ctkExpandButton::toggled,
          this,
          &QmitkGrowCutToolGUI::OnAdvancedSettingsButtonToggled);

  connect(m_Controls.m_distancePenaltyDoubleSpinBox,
          qOverload<double>(&QDoubleSpinBox::valueChanged),
          this,
          &QmitkGrowCutToolGUI::SetValueOfDistancePenaltySlider);

  connect(m_Controls.m_distancePenaltySlider,
          &QSlider::valueChanged,
          this,
          &QmitkGrowCutToolGUI::SetValueOfDistancePenaltyDoubleSpinBox);

  Superclass::InitializeUI(mainLayout);
}

void QmitkGrowCutToolGUI::SetValueOfDistancePenaltySlider(double val)
{
  m_Controls.m_distancePenaltySlider->setValue(val * 100);
}

void QmitkGrowCutToolGUI::SetValueOfDistancePenaltyDoubleSpinBox(int val)
{
  m_Controls.m_distancePenaltyDoubleSpinBox->setValue((static_cast<double>(val) / 100));
}

void QmitkGrowCutToolGUI::OnAdvancedSettingsButtonToggled(bool toggled)
{
  m_Controls.m_distancePenaltyLabel->setVisible(toggled);
  m_Controls.m_distancePenaltyDoubleSpinBox->setVisible(toggled);
  m_Controls.m_distancePenaltySlider->setVisible(toggled);

  double distancePenaltyMinium = 0.0;
  double distancePenaltyMaximum = 1.0;

  m_Controls.m_distancePenaltyDoubleSpinBox->setMinimum(distancePenaltyMinium);
  m_Controls.m_distancePenaltyDoubleSpinBox->setMaximum(distancePenaltyMaximum);
  m_Controls.m_distancePenaltySlider->setMinimum(distancePenaltyMinium);
  m_Controls.m_distancePenaltySlider->setMaximum(distancePenaltyMaximum * 100);

  m_Controls.m_distancePenaltyDoubleSpinBox->setSingleStep(0.01);
}

void QmitkGrowCutToolGUI::OnPreviewBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::GrowCutTool>();
  if (nullptr != tool)
  {
    try
    {
      tool->SetDistancePenalty(m_Controls.m_distancePenaltyDoubleSpinBox->value());

      tool->UpdatePreview();
    }
    catch (...)
    {
      this->setCursor(Qt::ArrowCursor);
      QMessageBox *messageBox =
        new QMessageBox(QMessageBox::Critical,
                        nullptr,
                        "itkGrowCutFilter error.");
      messageBox->exec();
      delete messageBox;
      return;
    }

    m_FirstPreviewComputation = true;
    this->SetLabelSetPreview(tool->GetPreviewSegmentation());
    tool->IsTimePointChangeAwareOn();
    this->ActualizePreviewLabelVisibility();
  }
}

void QmitkGrowCutToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);

  m_Controls.m_distancePenaltyLabel->setEnabled(enabled);
  m_Controls.m_distancePenaltyDoubleSpinBox->setEnabled(enabled);
  m_Controls.m_distancePenaltySlider->setEnabled(enabled);
}

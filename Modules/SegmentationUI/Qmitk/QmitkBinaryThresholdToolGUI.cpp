/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkBinaryThresholdToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <QApplication>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkBinaryThresholdToolGUI, "")

QmitkBinaryThresholdToolGUI::QmitkBinaryThresholdToolGUI()
  : QmitkToolGUI()
{
  // create the visible widgets
  QBoxLayout *mainLayout = new QVBoxLayout(this);

  QLabel *label = new QLabel("Threshold :", this);
  QFont f = label->font();
  f.setBold(false);
  label->setFont(f);
  mainLayout->addWidget(label);

  QBoxLayout *layout = new QHBoxLayout();

  m_ThresholdSlider = new ctkSliderWidget();
  connect(
    m_ThresholdSlider, SIGNAL(valueChanged(double)), this, SLOT(OnSliderValueChanged(double)));
  layout->addWidget(m_ThresholdSlider);
  mainLayout->addLayout(layout);
  m_ThresholdSlider->setSingleStep(0.01);

  QPushButton *okButton = new QPushButton("Confirm Segmentation", this);
  connect(okButton, SIGNAL(clicked()), this, SLOT(OnAcceptThresholdPreview()));
  okButton->setFont(f);
  mainLayout->addWidget(okButton);

  m_CheckProcessAll = new QCheckBox("Process all time steps", this);
  m_CheckProcessAll->setChecked(false);
  m_CheckProcessAll->setToolTip("Process/overwrite all time steps of the dynamic segmentation and not just the currently visible time step.");

  mainLayout->addWidget(m_CheckProcessAll);

  m_CheckCreateNew = new QCheckBox("Create as new segmentation", this);
  m_CheckCreateNew->setChecked(false);
  m_CheckCreateNew->setToolTip("Add the confirmed segmentation as a new segmentation instead of overwriting the currently selected.");
  mainLayout->addWidget(m_CheckCreateNew);

  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
}

QmitkBinaryThresholdToolGUI::~QmitkBinaryThresholdToolGUI()
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->CurrentlyBusy -=
      mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, bool>(this, &QmitkBinaryThresholdToolGUI::BusyStateChanged);
    m_BinaryThresholdTool->IntervalBordersChanged -=
      mitk::MessageDelegate3<QmitkBinaryThresholdToolGUI, double, double, bool>(
        this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged);
    m_BinaryThresholdTool->ThresholdingValuesChanged -= mitk::MessageDelegate2<QmitkBinaryThresholdToolGUI, mitk::ScalarType, mitk::ScalarType>(
      this, &QmitkBinaryThresholdToolGUI::OnThresholdingValuesChanged);
  }
}

void QmitkBinaryThresholdToolGUI::OnNewToolAssociated(mitk::Tool *tool)
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->CurrentlyBusy -=
      mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, bool>(this, &QmitkBinaryThresholdToolGUI::BusyStateChanged);
    m_BinaryThresholdTool->IntervalBordersChanged -=
      mitk::MessageDelegate3<QmitkBinaryThresholdToolGUI, double, double, bool>(
        this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged);
    m_BinaryThresholdTool->ThresholdingValuesChanged -= mitk::MessageDelegate2<QmitkBinaryThresholdToolGUI, mitk::ScalarType, mitk::ScalarType>(
      this, &QmitkBinaryThresholdToolGUI::OnThresholdingValuesChanged);
  }

  m_BinaryThresholdTool = dynamic_cast<mitk::BinaryThresholdTool *>(tool);

  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->CurrentlyBusy +=
      mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, bool>(this, &QmitkBinaryThresholdToolGUI::BusyStateChanged);
    m_BinaryThresholdTool->IntervalBordersChanged +=
      mitk::MessageDelegate3<QmitkBinaryThresholdToolGUI, double, double, bool>(
        this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged);
    m_BinaryThresholdTool->ThresholdingValuesChanged += mitk::MessageDelegate2<QmitkBinaryThresholdToolGUI, mitk::ScalarType, mitk::ScalarType>(
      this, &QmitkBinaryThresholdToolGUI::OnThresholdingValuesChanged);

    m_BinaryThresholdTool->SetOverwriteExistingSegmentation(true);
    m_CheckProcessAll->setVisible(m_BinaryThresholdTool->GetTargetSegmentationNode()->GetData()->GetTimeSteps()>1);
  }
}

void QmitkBinaryThresholdToolGUI::OnAcceptThresholdPreview()
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    if (m_CheckCreateNew->isChecked())
    {
      m_BinaryThresholdTool->SetOverwriteExistingSegmentation(false);
    }
    else
    {
      m_BinaryThresholdTool->SetOverwriteExistingSegmentation(true);
    }

    m_BinaryThresholdTool->SetCreateAllTimeSteps(m_CheckProcessAll->isChecked());

    this->thresholdAccepted();
    m_BinaryThresholdTool->ConfirmSegmentation();
  }
}

void QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged(double lower, double upper, bool isFloat)
{
  m_InternalUpdate = true;
  if (!isFloat)
  {
    m_ThresholdSlider->setRange(int(lower), int(upper));
    m_ThresholdSlider->setSingleStep(1);
    m_ThresholdSlider->setDecimals(0);
  }
  else
  {
    m_ThresholdSlider->setRange(lower, upper);
  }
  m_InternalUpdate = false;
}

void QmitkBinaryThresholdToolGUI::OnThresholdingValuesChanged(mitk::ScalarType lower, mitk::ScalarType /*upper*/)
{
  m_ThresholdSlider->setValue(lower);
}

void QmitkBinaryThresholdToolGUI::OnSliderValueChanged(double value)
{
  if (m_BinaryThresholdTool.IsNotNull() && !m_InternalUpdate)
  {
    m_BinaryThresholdTool->SetThresholdValue(value);
  }
}

void QmitkBinaryThresholdToolGUI::BusyStateChanged(bool value)
{
  if (value)
  {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }

  m_ThresholdSlider->setEnabled(!value);
}

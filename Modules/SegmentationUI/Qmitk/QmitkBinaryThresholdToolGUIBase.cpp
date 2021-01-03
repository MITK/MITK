/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkBinaryThresholdToolGUIBase.h"

#include "mitkBinaryThresholdBaseTool.h"
#include "mitkBinaryThresholdTool.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qslider.h>
#include <QApplication>

QmitkBinaryThresholdToolGUIBase::QmitkBinaryThresholdToolGUIBase(bool ulMode) : QmitkAutoSegmentationToolGUIBase(false), m_ULMode(ulMode)
{
}

QmitkBinaryThresholdToolGUIBase::~QmitkBinaryThresholdToolGUIBase()
{
  auto tool = this->GetConnectedToolAs<mitk::BinaryThresholdBaseTool>();

  if (nullptr != tool)
  {
    tool->IntervalBordersChanged -=
      mitk::MessageDelegate3<QmitkBinaryThresholdToolGUIBase, double, double, bool>(
        this, &QmitkBinaryThresholdToolGUIBase::OnThresholdingIntervalBordersChanged);
    tool->ThresholdingValuesChanged -=
      mitk::MessageDelegate2<QmitkBinaryThresholdToolGUIBase, mitk::ScalarType, mitk::ScalarType>(
        this, &QmitkBinaryThresholdToolGUIBase::OnThresholdingValuesChanged);
  }
}

void QmitkBinaryThresholdToolGUIBase::OnThresholdingIntervalBordersChanged(double lower, double upper, bool isFloat)
{
  m_InternalUpdate = true;

  if (m_ULMode)
  {
    if (!isFloat)
    {
      m_ThresholdRange->setRange(int(lower), int(upper));
      m_ThresholdRange->setSingleStep(1);
      m_ThresholdRange->setDecimals(0);
    }
    else
    {
      m_ThresholdRange->setRange(lower, upper);
    }
  }
  else
  {
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
  }

  m_InternalUpdate = false;
}

void QmitkBinaryThresholdToolGUIBase::OnThresholdingValuesChanged(mitk::ScalarType lower, mitk::ScalarType upper)
{
  if (m_ULMode)
  {
    m_ThresholdRange->setValues(lower, upper);
  }
  else
  {
    m_ThresholdSlider->setValue(lower);
  }
}

void QmitkBinaryThresholdToolGUIBase::OnThresholdRangeChanged(double min, double max)
{
  auto tool = this->GetConnectedToolAs<mitk::BinaryThresholdBaseTool>();

  if (nullptr != tool && !m_InternalUpdate)
  {
    tool->SetThresholdValues(min, max);
  }
}

void QmitkBinaryThresholdToolGUIBase::OnThresholdSliderChanged(double value)
{
  auto tool = this->GetConnectedToolAs<mitk::BinaryThresholdTool>();

  if (nullptr != tool && !m_InternalUpdate)
  {
    tool->SetThresholdValue(value);
  }
}

void QmitkBinaryThresholdToolGUIBase::DisconnectOldTool(mitk::AutoSegmentationWithPreviewTool* oldTool)
{
  Superclass::DisconnectOldTool(oldTool);

  auto tool = dynamic_cast<mitk::BinaryThresholdBaseTool*>(oldTool);

  if (nullptr != tool)
  {
    tool->IntervalBordersChanged -=
      mitk::MessageDelegate3<QmitkBinaryThresholdToolGUIBase, double, double, bool>(
        this, &QmitkBinaryThresholdToolGUIBase::OnThresholdingIntervalBordersChanged);
    tool->ThresholdingValuesChanged -=
      mitk::MessageDelegate2<QmitkBinaryThresholdToolGUIBase, mitk::ScalarType, mitk::ScalarType>(
        this, &QmitkBinaryThresholdToolGUIBase::OnThresholdingValuesChanged);
  }
}

void QmitkBinaryThresholdToolGUIBase::ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool)
{
  Superclass::ConnectNewTool(newTool);

  auto tool = dynamic_cast<mitk::BinaryThresholdBaseTool*>(newTool);

  if (nullptr != tool)
  {
    tool->IntervalBordersChanged +=
      mitk::MessageDelegate3<QmitkBinaryThresholdToolGUIBase, double, double, bool>(
        this, &QmitkBinaryThresholdToolGUIBase::OnThresholdingIntervalBordersChanged);
    tool->ThresholdingValuesChanged +=
      mitk::MessageDelegate2<QmitkBinaryThresholdToolGUIBase, mitk::ScalarType, mitk::ScalarType>(
        this, &QmitkBinaryThresholdToolGUIBase::OnThresholdingValuesChanged);
  }
}

void QmitkBinaryThresholdToolGUIBase::InitializeUI(QBoxLayout* mainLayout)
{
  QLabel* label = new QLabel("Threshold :", this);
  QFont f = label->font();
  f.setBold(false);
  label->setFont(f);
  mainLayout->addWidget(label);

  QBoxLayout* layout = new QHBoxLayout();

  if (m_ULMode)
  {
    m_ThresholdRange = new ctkRangeWidget();
    connect(
      m_ThresholdRange, SIGNAL(valuesChanged(double, double)), this, SLOT(OnThresholdRangeChanged(double, double)));
    layout->addWidget(m_ThresholdRange);
    m_ThresholdRange->setSingleStep(0.01);
  }
  else
  {
    m_ThresholdSlider = new ctkSliderWidget();
    connect(
      m_ThresholdSlider, SIGNAL(valueChanged(double)), this, SLOT(OnThresholdSliderChanged(double)));
    layout->addWidget(m_ThresholdSlider);
    m_ThresholdSlider->setSingleStep(0.01);
  }

  mainLayout->addLayout(layout);

  Superclass::InitializeUI(mainLayout);
}

void QmitkBinaryThresholdToolGUIBase::BusyStateChanged(bool value)
{
  Superclass::BusyStateChanged(value);

  if (m_ThresholdRange)
  {
    m_ThresholdRange->setEnabled(!value);
  }

  if (m_ThresholdSlider)
  {
    m_ThresholdSlider->setEnabled(!value);
  }
}

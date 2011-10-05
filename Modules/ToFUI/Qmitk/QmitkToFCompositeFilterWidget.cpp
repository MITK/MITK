/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-20 13:35:09 +0200 (Mi, 20 Mai 2009) $
Version:   $Revision: 17332 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <QmitkToFCompositeFilterWidget.h>

//QT headers
#include <QPlastiqueStyle>

const std::string QmitkToFCompositeFilterWidget::VIEW_ID = "org.mitk.views.qmitktofcompositefilterwidget";

QmitkToFCompositeFilterWidget::QmitkToFCompositeFilterWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  this->m_ToFCompositeFilter = NULL;

  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkToFCompositeFilterWidget::~QmitkToFCompositeFilterWidget()
{
}

void QmitkToFCompositeFilterWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToFCompositeFilterWidgetControls;
    m_Controls->setupUi(parent);

    QPlastiqueStyle *sliderStyle = new QPlastiqueStyle();

    int min = m_Controls->m_ThresholdFilterMinValueSpinBox->value();
    int max = m_Controls->m_ThresholdFilterMaxValueSpinBox->value();
    m_Controls->m_ThresholdFilterRangeSlider->setMinimum(min);
    m_Controls->m_ThresholdFilterRangeSlider->setMaximum(max);
    m_Controls->m_ThresholdFilterRangeSlider->setLowerValue(min);
    m_Controls->m_ThresholdFilterRangeSlider->setUpperValue(max);
    m_Controls->m_ThresholdFilterRangeSlider->setHandleMovementMode(QxtSpanSlider::NoOverlapping);
    m_Controls->m_ThresholdFilterRangeSlider->setStyle(sliderStyle);

    this->CreateConnections();
  }
}

void QmitkToFCompositeFilterWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect(m_Controls->m_TemporalMedianFilterNumOfFramesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnTemporalMedianFilterNumOfFramesSpinBoxValueChanged(int)));
    connect(m_Controls->m_BilateralFilterDomainSigmaSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnBilateralFilterDomainSigmaSpinBoxValueChanged(double)));
    connect(m_Controls->m_BilateralFilterRangeSigmaSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnBilateralFilterRangeSigmaSpinBoxValueChanged(double)));
    connect(m_Controls->m_BilateralFilterKernelRadiusSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnBilateralFilterKernelRadiusSpinBoxValueChanged(int)));
    connect(m_Controls->m_ThresholdFilterMinValueSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnThresholdFilterMinValueChanged(int)));
    connect(m_Controls->m_ThresholdFilterMaxValueSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnThresholdFilterMaxValueChanged(int)));

    connect( (QObject*)(m_Controls->m_TemporalMedianFilterCheckBox), SIGNAL(toggled(bool)), this, SLOT(OnTemporalMedianFilterCheckBoxChecked(bool)) );
    connect( (QObject*)(m_Controls->m_AverageFilterCheckBox), SIGNAL(toggled(bool)), this, SLOT(OnAverageFilterCheckBoxChecked(bool)) );
    connect( (QObject*)(m_Controls->m_ThresholdFilterCheckBox), SIGNAL(toggled(bool)), this, SLOT(OnThresholdFilterCheckBoxChecked(bool)) );
    connect( (QObject*)(m_Controls->m_BilateralFilterCheckBox), SIGNAL(toggled(bool)), this, SLOT(OnBilateralFilterCheckBoxChecked(bool)) );
    connect( (QObject*)(m_Controls->m_MedianFilterCheckBox), SIGNAL(toggled(bool)), this, SLOT(OnMedianFilterCheckBoxChecked(bool)) );

    connect(m_Controls->m_ThresholdFilterRangeSlider, SIGNAL(spanChanged(int, int)  ),this, SLOT( OnSpanChanged(int , int ) ));

    //reset button
    connect(m_Controls->m_ThresholdFilterRangeSliderReset, SIGNAL(pressed()), this, SLOT(OnResetThresholdFilterRangeSlider()));

  }
}

void QmitkToFCompositeFilterWidget::SetToFCompositeFilter(mitk::ToFCompositeFilter* toFCompositeFilter)
{
  this->m_ToFCompositeFilter = toFCompositeFilter;
}

mitk::ToFCompositeFilter* QmitkToFCompositeFilterWidget::GetToFCompositeFilter()
{
  if (this->m_ToFCompositeFilter.IsNull())
  {
    this->m_ToFCompositeFilter = mitk::ToFCompositeFilter::New();
  }
  return this->m_ToFCompositeFilter;
}

void QmitkToFCompositeFilterWidget::UpdateFilterParameter()
{
  OnTemporalMedianFilterCheckBoxChecked(m_Controls->m_TemporalMedianFilterCheckBox->isChecked());
  OnAverageFilterCheckBoxChecked(m_Controls->m_AverageFilterCheckBox->isChecked());
  OnMedianFilterCheckBoxChecked(m_Controls->m_MedianFilterCheckBox->isChecked());
  OnThresholdFilterCheckBoxChecked(m_Controls->m_ThresholdFilterCheckBox->isChecked());
  OnBilateralFilterCheckBoxChecked(m_Controls->m_BilateralFilterCheckBox->isChecked());
}

void QmitkToFCompositeFilterWidget::OnTemporalMedianFilterCheckBoxChecked(bool checked)
{
  this->m_ToFCompositeFilter->SetApplyTemporalMedianFilter(checked);
  // disable average filter if temporal median filter is enabled
  if (checked)
  {
    m_Controls->m_AverageFilterCheckBox->setChecked(false);
    this->m_ToFCompositeFilter->SetApplyAverageFilter(false);
  }
}

void QmitkToFCompositeFilterWidget::OnAverageFilterCheckBoxChecked(bool checked)
{
  this->m_ToFCompositeFilter->SetApplyAverageFilter(checked);
  // disable temporal median filter if average filter is enabled
  if (checked)
  {
    m_Controls->m_TemporalMedianFilterCheckBox->setChecked(false);
    this->m_ToFCompositeFilter->SetApplyTemporalMedianFilter(false);
  }
}

void QmitkToFCompositeFilterWidget::OnThresholdFilterCheckBoxChecked(bool checked)
{
  this->m_ToFCompositeFilter->SetApplyThresholdFilter(checked);
}

void QmitkToFCompositeFilterWidget::OnMedianFilterCheckBoxChecked(bool checked)
{
  this->m_ToFCompositeFilter->SetApplyMedianFilter(checked);
}

void QmitkToFCompositeFilterWidget::OnBilateralFilterCheckBoxChecked(bool checked)
{
  this->m_ToFCompositeFilter->SetApplyBilateralFilter(checked);
}

void QmitkToFCompositeFilterWidget::OnTemporalMedianFilterNumOfFramesSpinBoxValueChanged(int value)
{
  this->m_ToFCompositeFilter->SetTemporalMedianFilterParameter(value);
}

void QmitkToFCompositeFilterWidget::OnBilateralFilterDomainSigmaSpinBoxValueChanged(double value)
{
  SetBilateralFilterParameter();
}

void QmitkToFCompositeFilterWidget::OnBilateralFilterRangeSigmaSpinBoxValueChanged(double value)
{
  SetBilateralFilterParameter();
}

void QmitkToFCompositeFilterWidget::OnBilateralFilterKernelRadiusSpinBoxValueChanged(int value)
{
  SetBilateralFilterParameter();
}

void QmitkToFCompositeFilterWidget::OnThresholdFilterMinValueChanged(int value)
{
  m_Controls->m_ThresholdFilterRangeSlider->setLowerValue(value);
  SetThresholdFilterParameter();
}

void QmitkToFCompositeFilterWidget::OnThresholdFilterMaxValueChanged(int value)
{
  m_Controls->m_ThresholdFilterRangeSlider->setUpperValue(value);
  SetThresholdFilterParameter();
}


void QmitkToFCompositeFilterWidget::SetThresholdFilterParameter()
{
  int min = m_Controls->m_ThresholdFilterMinValueSpinBox->value();
  int max = m_Controls->m_ThresholdFilterMaxValueSpinBox->value();
  this->m_ToFCompositeFilter->SetThresholdFilterParameter(min, max);
}

void QmitkToFCompositeFilterWidget::SetBilateralFilterParameter()
{
  double domainSigma = m_Controls->m_BilateralFilterDomainSigmaSpinBox->value();
  double rangeSigma = m_Controls->m_BilateralFilterRangeSigmaSpinBox->value();
  int kernelRadius = m_Controls->m_BilateralFilterKernelRadiusSpinBox->value();
  this->m_ToFCompositeFilter->SetBilateralFilterParameter(domainSigma, rangeSigma, kernelRadius);
}

void QmitkToFCompositeFilterWidget::OnSpanChanged(int lower, int upper) 
{
  int lowerVal =  m_Controls->m_ThresholdFilterRangeSlider->lowerValue();
  int upperVal =  m_Controls->m_ThresholdFilterRangeSlider->upperValue();

  m_Controls->m_ThresholdFilterMinValueSpinBox->setValue(lowerVal);
  m_Controls->m_ThresholdFilterMaxValueSpinBox->setValue(upperVal);
}

void QmitkToFCompositeFilterWidget::OnResetThresholdFilterRangeSlider() 
{
  int lower =  1;
  int upper =  7000;

  m_Controls->m_ThresholdFilterRangeSlider->setLowerValue(lower);
  m_Controls->m_ThresholdFilterRangeSlider->setUpperValue(upper);

  m_Controls->m_ThresholdFilterMinValueSpinBox->setValue(lower);
  m_Controls->m_ThresholdFilterMaxValueSpinBox->setValue(upper);
}

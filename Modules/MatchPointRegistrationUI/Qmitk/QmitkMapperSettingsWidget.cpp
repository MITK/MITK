/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMapperSettingsWidget.h"

#include <QmitkMappingJob.h>

#include <ui_QmitkMapperSettingsWidget.h>

QmitkMapperSettingsWidget::QmitkMapperSettingsWidget(QWidget *parent)
  : QWidget(parent),
    m_Controls(std::make_unique<Ui::QmitkMapperSettingsWidget>()),
    m_MaskMode(false),
    m_allowSampling(true)
{
  m_Controls->setupUi(this);

  connect(m_Controls->m_cbLinkFactors, SIGNAL(clicked()), this, SLOT(OnLinkSampleFactorChecked()));
  connect(m_Controls->m_sbXFactor, SIGNAL(valueChanged(double)), this, SLOT(OnXFactorChanged(double)));
}

QmitkMapperSettingsWidget::~QmitkMapperSettingsWidget()
{
}

void QmitkMapperSettingsWidget::ConfigureJobSettings(QmitkMappingJobSettings *settings)
{
  if (!settings)
  {
    return;
  }

  settings->m_allowUndefPixels = m_Controls->m_groupAllowUndefPixels->isChecked();
  settings->m_paddingValue = m_Controls->m_sbPaddingValue->value();
  settings->m_allowUnregPixels = m_Controls->m_groupAllowUnregPixels->isChecked();
  settings->m_errorValue = m_Controls->m_sbErrorValue->value();
  settings->m_InterpolatorLabel = m_Controls->m_comboInterpolator->currentText().toStdString();

  switch (m_Controls->m_comboInterpolator->currentIndex())
  {
    case 0:
      settings->m_InterpolatorType = mitk::ImageMappingInterpolator::NearestNeighbor;
      break;

    case 1:
      settings->m_InterpolatorType = mitk::ImageMappingInterpolator::Linear;
      break;

    case 2:
      settings->m_InterpolatorType = mitk::ImageMappingInterpolator::BSpline_3;
      break;

    case 3:
      settings->m_InterpolatorType = mitk::ImageMappingInterpolator::WSinc_Hamming;
      break;

    case 4:
      settings->m_InterpolatorType = mitk::ImageMappingInterpolator::WSinc_Welch;
      break;
  }
}

void QmitkMapperSettingsWidget::OnXFactorChanged(double d)
{
  if (m_Controls->m_cbLinkFactors->isChecked())
  {
    m_Controls->m_sbYFactor->setValue(d);
    m_Controls->m_sbZFactor->setValue(d);
  }
}

void QmitkMapperSettingsWidget::OnLinkSampleFactorChecked()
{
  m_Controls->m_sbYFactor->setEnabled(!(m_Controls->m_cbLinkFactors->isChecked()));
  m_Controls->m_sbZFactor->setEnabled(!(m_Controls->m_cbLinkFactors->isChecked()));

  if (m_Controls->m_cbLinkFactors->isChecked())
  {
    m_Controls->m_sbYFactor->setValue(m_Controls->m_sbXFactor->value());
    m_Controls->m_sbZFactor->setValue(m_Controls->m_sbXFactor->value());
  }
}

void QmitkMapperSettingsWidget::SetMaskMode(bool activeMask)
{
  if (activeMask != m_MaskMode)
  {
    if (activeMask)
    {
      m_Controls->m_comboInterpolator->setCurrentIndex(0);
      m_Controls->m_sbErrorValue->setValue(0);
      m_Controls->m_sbPaddingValue->setValue(0);
    }
    else
    {
      m_Controls->m_comboInterpolator->setCurrentIndex(1);
    }
  }

  m_MaskMode = activeMask;
}

void QmitkMapperSettingsWidget::AllowSampling(bool allow)
{
  m_allowSampling = allow;
  m_Controls->m_groupActivateSampling->setVisible(allow);
}

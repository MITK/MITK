/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkMapperSettingsWidget.h"

#include <QmitkMappingJob.h>

QmitkMapperSettingsWidget::QmitkMapperSettingsWidget(QWidget *parent) : QWidget(parent), m_MaskMode(false), m_allowSampling(true)
{
  this->setupUi(this);

  connect(m_cbLinkFactors, SIGNAL(clicked()), this, SLOT(OnLinkSampleFactorChecked()));
  connect(m_sbXFactor, SIGNAL(valueChanged(double)), this, SLOT(OnXFactorChanged(double)));
}

void QmitkMapperSettingsWidget::ConfigureJobSettings(QmitkMappingJobSettings *settings)
{
  if (!settings)
  {
    return;
  }

  settings->m_allowUndefPixels = m_groupAllowUndefPixels->isChecked();
  settings->m_paddingValue = m_sbPaddingValue->value();
  settings->m_allowUnregPixels = m_groupAllowUnregPixels->isChecked();
  settings->m_errorValue = m_sbErrorValue->value();
  settings->m_InterpolatorLabel = m_comboInterpolator->currentText().toStdString();

  switch (m_comboInterpolator->currentIndex())
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
  if (m_cbLinkFactors->isChecked())
  {
    this->m_sbYFactor->setValue(d);
    this->m_sbZFactor->setValue(d);
  }
}

void QmitkMapperSettingsWidget::OnLinkSampleFactorChecked()
{
  this->m_sbYFactor->setEnabled(!(this->m_cbLinkFactors->isChecked()));
  this->m_sbZFactor->setEnabled(!(this->m_cbLinkFactors->isChecked()));

  if (m_cbLinkFactors->isChecked())
  {
    this->m_sbYFactor->setValue(this->m_sbXFactor->value());
    this->m_sbZFactor->setValue(this->m_sbXFactor->value());
  }
}

void QmitkMapperSettingsWidget::SetMaskMode(bool activeMask)
{
  if (activeMask != m_MaskMode)
  {
    if (activeMask)
    {
      this->m_comboInterpolator->setCurrentIndex(0);
      this->m_sbErrorValue->setValue(0);
      this->m_sbPaddingValue->setValue(0);
    }
    else
    {
      this->m_comboInterpolator->setCurrentIndex(1);
    }
  }

  m_MaskMode = activeMask;
}

void QmitkMapperSettingsWidget::AllowSampling(bool allow)
{
  m_allowSampling = allow;
  m_groupActivateSampling->setVisible(allow);
}

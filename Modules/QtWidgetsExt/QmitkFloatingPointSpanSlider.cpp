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


#include "QmitkFloatingPointSpanSlider.h"
#include <QObject>

//#include <mitkCommon.h>

QmitkFloatingPointSpanSlider::QmitkFloatingPointSpanSlider(QWidget *parent)
  : QxtSpanSlider(parent), m_LowerValue(0), m_UpperValue(1000),
    m_Minimum(0), m_Maximum(1000), offset(0.0), factor(1.0), m_IntMode(false)
{
  this->QxtSpanSlider::setMinimum(0);
  this->QxtSpanSlider::setMaximum(1000);

  connect(this, SIGNAL(spanChanged(int,int)), this, SLOT(IntSpanChanged(int,int)));
}

void QmitkFloatingPointSpanSlider::setIntegerMode(bool intMode)
{
  m_IntMode = intMode;
  this->QxtSpanSlider::setMinimum(m_Minimum);
  this->QxtSpanSlider::setMaximum(m_Maximum);
}

double QmitkFloatingPointSpanSlider::lowerValue() const
{
  if (m_IntMode) return this->QxtSpanSlider::lowerValue();
  return m_LowerValue;
}

double QmitkFloatingPointSpanSlider::upperValue() const
{
  if (m_IntMode) return this->QxtSpanSlider::upperValue();
  return m_UpperValue;
}

void QmitkFloatingPointSpanSlider::setLowerValue(double lower)
{
  m_LowerValue = lower;
  if (m_IntMode)
  {
    this->QxtSpanSlider::setLowerValue(lower);
  }
  else
  {
    this->QxtSpanSlider::setLowerValue(scaleValue(lower));
    emit lowerValueChanged(lower);
  }
}

void QmitkFloatingPointSpanSlider::setUpperValue(double upper)
{
  m_UpperValue = upper;
  if (m_IntMode)
  {
    this->QxtSpanSlider::setUpperValue(upper);
  }
  else
  {
    this->QxtSpanSlider::setUpperValue(scaleValue(upper));
    emit upperValueChanged(upper);
  }
}

void QmitkFloatingPointSpanSlider::setSpan(double lower, double upper)
{
  //MITK_INFO << "QmitkFloatingPointSpanSlider new span: " << lower << " - " << upper;
  m_LowerValue = lower;
  m_UpperValue = upper;
  if (m_IntMode)
  {
    this->QxtSpanSlider::setLowerValue(lower);
    this->QxtSpanSlider::setUpperValue(upper);
  }
  else
  {
    this->QxtSpanSlider::setLowerValue(scaleValue(lower));
    this->QxtSpanSlider::setUpperValue(scaleValue(upper));
    emit spanChanged(lower, upper);
  }
}

double QmitkFloatingPointSpanSlider::maximum() const
{
  if (m_IntMode) return this->QxtSpanSlider::maximum();
  return m_Maximum;
}

double QmitkFloatingPointSpanSlider::minimum() const
{
  if (m_IntMode) return this->QxtSpanSlider::minimum();
  return m_Minimum;
}

void QmitkFloatingPointSpanSlider::setMaximum(double max)
{
  //MITK_INFO << "QmitkFloatingPointSpanSlider new max: " << max;
  if (m_IntMode)
  {
    this->QxtSpanSlider::setMaximum(max);
    return;
  }

  if (max < m_Minimum)
  {
    m_Minimum = max;
  }
  m_Maximum = max;
  scaleSliderToInt();

  if (m_LowerValue > m_Maximum)
  {
    m_LowerValue = m_Maximum;
    this->QxtSpanSlider::setLowerValue(scaleValue(m_LowerValue));
  }
  if (m_UpperValue > m_Maximum)
  {
    m_UpperValue = m_Maximum;
    this->QxtSpanSlider::setUpperValue(scaleValue(m_UpperValue));
  }
}

void QmitkFloatingPointSpanSlider::setMinimum(double min)
{
  //MITK_INFO << "QmitkFloatingPointSpanSlider new min: " << min;
  if (m_IntMode)
  {
    this->QxtSpanSlider::setMinimum(min);
    return;
  }

  if (min > m_Maximum)
  {
    m_Maximum = min;
  }
  m_Minimum = min;
  scaleSliderToInt();

  if (m_LowerValue < m_Minimum)
  {
    m_LowerValue = m_Minimum;
    this->QxtSpanSlider::setLowerValue(scaleValue(m_LowerValue));
  }
  if (m_UpperValue < m_Minimum)
  {
    m_UpperValue = m_Minimum;
    this->QxtSpanSlider::setUpperValue(scaleValue(m_UpperValue));
  }
}

void QmitkFloatingPointSpanSlider::setRange(double min, double max)
{
  if (m_IntMode)
  {
    this->QxtSpanSlider::setRange(min, max);
    m_Minimum = minimum();
    m_Maximum = maximum();
    m_LowerValue = lowerValue();
    m_UpperValue = upperValue();
    return;
  }

  if (min > max)
  {
    min = max;
  }
  m_Minimum = min;
  m_Maximum = max;

  if (m_LowerValue > m_Maximum)
  {
    m_LowerValue = m_Maximum;
    this->QxtSpanSlider::setLowerValue(scaleValue(m_LowerValue));
  }
  else if (m_LowerValue < m_Minimum)
  {
    m_LowerValue = m_Minimum;
    this->QxtSpanSlider::setLowerValue(scaleValue(m_LowerValue));
  }

  if (m_UpperValue > m_Maximum)
  {
    m_UpperValue = m_Maximum;
    this->QxtSpanSlider::setUpperValue(scaleValue(m_UpperValue));
  }
  else if (m_UpperValue < m_Minimum)
  {
    m_UpperValue = m_Minimum;
    this->QxtSpanSlider::setUpperValue(scaleValue(m_UpperValue));
  }

  scaleSliderToInt();
}

void QmitkFloatingPointSpanSlider::IntSpanChanged(int lower, int upper)
{
  if (m_IntMode)
  {
    m_LowerValue = lower;
    m_UpperValue = upper;
  }
  else
  {
    m_LowerValue = unscaleValue(lower);
    m_UpperValue = unscaleValue(upper);
  }

  emit spanChanged(m_LowerValue, m_UpperValue);
}

void QmitkFloatingPointSpanSlider::scaleSliderToInt()
{
  int tmpLower = unscaleValue(m_LowerValue);
  int tmpUpper = unscaleValue(m_UpperValue);

  double range = m_Maximum - m_Minimum;
  factor = range ? 1000.0 / range : 0;
  offset = -m_Minimum;

  //MITK_INFO << "New offset: " << offset << ", scale: " << factor;

  m_LowerValue = scaleValue(tmpLower);
  m_UpperValue = scaleValue(tmpUpper);
}

int QmitkFloatingPointSpanSlider::scaleValue(double val)
{
  int scaled = factor ? (offset + val)*factor : 0;
  //MITK_INFO << "Scaling " << val << " (double) ==> " << scaled << " (int)";
  return scaled;
}

double QmitkFloatingPointSpanSlider::unscaleValue(int val)
{
  double unscaled = factor ? static_cast<double>(val)/factor - offset : 0;
  //MITK_INFO << "Unscaling " << val << " (int) ==> " << unscaled << " (double)";
  return unscaled;
}

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSliceNavigationWidget.h"

QmitkSliceNavigationWidget::QmitkSliceNavigationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  this->setupUi(this);

  m_Slider->setOrientation(Qt::Horizontal);
  m_Slider->setMinimum(0);
  m_Slider->setMaximum(0);
  m_Slider->setValue(0);
  m_Slider->setSingleStep(1);
  m_Slider->setPageStep(10);

  m_SpinBox->setMinimum( 0 );
  m_SpinBox->setMaximum(0);
  m_SpinBox->setValue(0);
  m_SpinBox->setDecimals(0);
  m_SpinBox->setSingleStep(1);

  this->connect(m_Slider, SIGNAL(valueChanged(double)), SLOT(SliderChanged(double)));
  this->connect(m_SpinBox, SIGNAL(valueChanged(double)), SLOT(SpinBoxChanged(double)));

  // this avoids trying to use m_Stepper until it is set to something != nullptr
  // (additionally to the avoiding recursions during refetching)
  m_InRefetch = true;

  // Hide slider labeling -- until it is explicitly activated
  this->ShowLabels(false);

  // Set label values as invalid (N/A)
  this->SetLabelValuesValid(false, false);

  m_HasLabels = false;
  m_HasLabelUnit = true;
  m_InverseDirection = false;
  m_InvertedControls = false;
}

QString QmitkSliceNavigationWidget::ClippedValueToString(float value)
{
  if (value < -10000000.0)
  {
    return "-INF";
  }
  else if (value > 10000000.0)
  {
    return "+INF";
  }
  else
  {
    return QString::number(value, 'f', 2);
  }
}

QString QmitkSliceNavigationWidget::GetLabelUnit()
{
  return m_LabelUnit;
}

QString QmitkSliceNavigationWidget::GetMinValueLabel()
{
  if (m_MinValueValid)
  {
    return this->ClippedValueToString(m_MinValue);
  }
  else
  {
    return "N/A";
  }
}

QString QmitkSliceNavigationWidget::GetMaxValueLabel()
{
  if (m_MaxValueValid)
  {
    return this->ClippedValueToString(m_MaxValue);
  }
  else
  {
    return "N/A";
  }
}

int QmitkSliceNavigationWidget::GetPos()
{
  return m_Stepper->GetPos();
}

bool QmitkSliceNavigationWidget::GetInverseDirection() const
{
  return m_InverseDirection;
}

bool QmitkSliceNavigationWidget::GetInvertedControls() const
{
  return m_InvertedControls;
}

void QmitkSliceNavigationWidget::Refetch()
{
  if (!m_InRefetch)
  {
    m_InRefetch = true;

    if (m_Stepper->GetSteps() == 0)
    {
      m_Slider->setMaximum(0);
      m_Slider->setValue(0);
      m_SpinBox->setMaximum(0);
      m_SpinBox->setValue(0);
    }
    else
    {
      unsigned int maximumSteps = m_Stepper->GetSteps() - 1;
      unsigned int currentPosition = m_Stepper->GetPos();

      // set slider value
      m_Slider->setMaximum(maximumSteps);
      if (m_InverseDirection)
      {
        m_Slider->setValue(maximumSteps - currentPosition);
      }
      else
      {
        m_Slider->setValue(currentPosition);
      }

      // set spinbox values
      m_SpinBox->setMaximum(maximumSteps);
      if (m_InverseDirection)
      {
        m_SpinBox->setValue(maximumSteps - currentPosition);
      }
      else
      {
        m_SpinBox->setValue(currentPosition);
      }
    }

    if (m_Stepper->HasRange() && m_HasLabels)
    {
      // Show slider with labels according to settings below
      m_SliderLabelLeft->setHidden(false);
      m_SliderLabelRight->setHidden(false);

      if (m_Stepper->HasValidRange())
      {
        this->SetLabelValuesValid(true, true);
        this->SetLabelValues(m_Stepper->GetRangeMin(), m_Stepper->GetRangeMax());
      }
      else
      {
        this->SetLabelValuesValid(false, false);
      }

      if (m_Stepper->HasUnitName())
      {
        this->SetLabelUnit(m_Stepper->GetUnitName());
      }
    }
    else
    {
      // Show slider without any labels
      m_SliderLabelLeft->setHidden(true);
      m_SliderLabelRight->setHidden(true);
    }

    // Update GUI according to settings above
    this->SetLabels();

    m_InRefetch = false;
  }
}

void QmitkSliceNavigationWidget::SetStepper(mitk::Stepper* stepper)
{
  m_Stepper = stepper;

  // this avoids trying to use m_Stepper until it is set to something != nullptr
  // (additionally to the avoiding recursions during refetching)
  m_InRefetch = (stepper == nullptr);
}

void QmitkSliceNavigationWidget::ShowLabels(bool show)
{
  m_HasLabels = show;
}

void QmitkSliceNavigationWidget::ShowLabelUnit(bool show)
{
  m_HasLabelUnit = show;
}

void QmitkSliceNavigationWidget::SetPos(int val)
{
  if (!m_InRefetch)
  {
    m_Stepper->SetPos(val);
  }
}

void QmitkSliceNavigationWidget::SetInverseDirection(bool inverseDirection)
{
  if (inverseDirection != m_InverseDirection)
  {
    m_InverseDirection = inverseDirection;
    this->Refetch();
  }
}

void QmitkSliceNavigationWidget::SetInvertedControls(bool invertedControls)
{
  if (invertedControls != m_InvertedControls)
  {
    m_InvertedControls = invertedControls;
    m_Slider->setInvertedAppearance(invertedControls);
    m_Slider->setInvertedControls(invertedControls);
    m_SpinBox->setInvertedControls(invertedControls);
  }
}

void QmitkSliceNavigationWidget::SliderChanged(double)
{
  if (m_InRefetch)
  {
    return;
  }

  if (m_InverseDirection)
  {
    m_Stepper->SetPos(m_Stepper->GetSteps() - 1 - m_Slider->value());
  }
  else
  {
    m_Stepper->SetPos(m_Slider->value());
  }

  this->Refetch();
}

void QmitkSliceNavigationWidget::SpinBoxChanged(double)
{
  if (m_InRefetch)
  {
    return;
  }

  if (m_InverseDirection)
  {
    m_Stepper->SetPos(m_Stepper->GetSteps() - 1 - m_SpinBox->value());
  }
  else
  {
    m_Stepper->SetPos(m_SpinBox->value());
  }

  this->Refetch();
}

void QmitkSliceNavigationWidget::SetLabelValues(float min, float max)
{
  m_MinValue = min;
  m_MaxValue = max;
}

void QmitkSliceNavigationWidget::SetLabelValuesValid(bool minValid, bool maxValid)
{
  m_MinValueValid = minValid;
  m_MaxValueValid = maxValid;
}

void QmitkSliceNavigationWidget::SetLabelUnit(const char* unit)
{
  m_LabelUnit = unit;
}

void QmitkSliceNavigationWidget::SetLabels()
{
  QString minText = "<p align='center'><font size='2'>" + this->GetMinValueLabel();

  QString maxText = "<p align='center'><font size='2'>" + this->GetMaxValueLabel();

  if (m_HasLabelUnit)
  {
    minText += "&nbsp;" + this->GetLabelUnit();
    maxText += "&nbsp;" + this->GetLabelUnit();
  }

  if (m_MinValueValid)
  {
    minText += "<br>(pos&nbsp;0)";
  }

  if (m_MaxValueValid)
  {
    maxText += "<br>(pos&nbsp;" + QString::number(m_Stepper->GetSteps() - 1) + ")";
  }

  minText += "</font></p>";
  maxText += "</font></p>";

  m_SliderLabelLeft->setText(minText);
  m_SliderLabelRight->setText(maxText);
}

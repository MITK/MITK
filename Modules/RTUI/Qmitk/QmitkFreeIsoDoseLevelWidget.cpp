/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkFreeIsoDoseLevelWidget.h"


QmitkFreeIsoDoseLevelWidget::QmitkFreeIsoDoseLevelWidget(QWidget*): m_ReferenceDose (40.0), m_InternalUpdate(false)
{
  this->setupUi(this);

  this->colorBtn->setDisplayColorName(false);
  this->m_IsoDoseLevel = mitk::IsoDoseLevel::New();

  connect(this->sbAbsValue, SIGNAL(valueChanged(double)), this, SLOT(OnAbsValueChanged(double)));
  connect(this->sbRelValue, SIGNAL(valueChanged(double)), this, SLOT(OnRelValueChanged(double)));
  connect(this->doseSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderChanged(int)));
  connect(this->checkVisibleIso, SIGNAL(clicked(bool)), this, SLOT(OnVisibleClicked(bool)));
  connect(this->colorBtn, SIGNAL(colorChanged(QColor)), this, SLOT(OnColorChanged(QColor)));
}

mitk::DoseValueAbs
  QmitkFreeIsoDoseLevelWidget::
  getReferenceDose() const
{
  return this->m_ReferenceDose;
};

mitk::IsoDoseLevel*
  QmitkFreeIsoDoseLevelWidget::
  getIsoDoseLevel() const
{
  return this->m_IsoDoseLevel;
};

void QmitkFreeIsoDoseLevelWidget::
  setReferenceDose(double newReferenceDose)
{
  if (newReferenceDose != m_ReferenceDose)
  {
    this->m_ReferenceDose = newReferenceDose;
    this->update();
  }
};

void QmitkFreeIsoDoseLevelWidget::
  setIsoDoseLevel(mitk::IsoDoseLevel* level)
{
  if (level != m_IsoDoseLevel)
  {
    if(!level)
    {
      mitkThrow() << "Error. Cannot set iso dose level for widget to nullptr pointer.";
    }

    this->m_IsoDoseLevel = level;
    this->update();
  }
}

void QmitkFreeIsoDoseLevelWidget::
  OnRelValueChanged(double newValue)
{
  if(!m_InternalUpdate)
  {
    updateValue(newValue/100.0);
  }
};

void QmitkFreeIsoDoseLevelWidget::
  OnAbsValueChanged(double newValue)
{
  if(!m_InternalUpdate)
  {
    updateValue(newValue/this->m_ReferenceDose);
  }
};

void QmitkFreeIsoDoseLevelWidget::
  OnSliderChanged(int newValue)
{
  if(!m_InternalUpdate)
  {
    updateValue(newValue/100.0);
  }
};

void QmitkFreeIsoDoseLevelWidget::
  OnVisibleClicked(bool checked)
{
  this->m_IsoDoseLevel->SetVisibleIsoLine(checked);
  emit VisualizationStyleChanged(this->m_IsoDoseLevel);
};

void QmitkFreeIsoDoseLevelWidget::
  OnColorChanged(QColor color)
{
  mitk::IsoDoseLevel::ColorType doseColor;
  doseColor.SetRed(color.redF());
  doseColor.SetGreen(color.greenF());
  doseColor.SetBlue(color.blueF());
  this->m_IsoDoseLevel->SetColor(doseColor);
  emit ColorChanged(this->m_IsoDoseLevel);
};

void QmitkFreeIsoDoseLevelWidget::
  updateValue(mitk::DoseValueRel newDose)
{
  m_InternalUpdate = true;

  mitk::DoseValueRel oldValue = this->m_IsoDoseLevel->GetDoseValue();
  this->m_IsoDoseLevel->SetDoseValue(newDose);
  this->sbAbsValue->setValue(newDose*this->m_ReferenceDose);
  this->sbRelValue->setValue(newDose*100);
  this->doseSlider->setValue(newDose*100);

  m_InternalUpdate = false;

  emit ValueChanged(this->m_IsoDoseLevel,oldValue);
};

void QmitkFreeIsoDoseLevelWidget::
  update()
{
  updateValue(this->m_IsoDoseLevel->GetDoseValue());

  this->checkVisibleIso->setChecked(this->m_IsoDoseLevel->GetVisibleIsoLine());

  QColor color;
  color.setRgbF(this->m_IsoDoseLevel->GetColor().GetRed(),this->m_IsoDoseLevel->GetColor().GetGreen(),this->m_IsoDoseLevel->GetColor().GetBlue());
  this->colorBtn->setColor(color);
};

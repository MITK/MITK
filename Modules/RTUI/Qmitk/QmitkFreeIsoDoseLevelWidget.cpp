/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFreeIsoDoseLevelWidget.h"
#include <ui_QmitkFreeIsoDoseLevelWidget.h>

QmitkFreeIsoDoseLevelWidget::QmitkFreeIsoDoseLevelWidget(QWidget*)
  : m_Controls(std::make_unique<Ui::QmitkFreeIsoDoseLevelWidget>()),
    m_ReferenceDose (40.0),
    m_InternalUpdate(false)
{
  m_Controls->setupUi(this);

  m_Controls->colorBtn->setDisplayColorName(false);
  m_IsoDoseLevel = mitk::IsoDoseLevel::New();

  connect(m_Controls->sbAbsValue, SIGNAL(valueChanged(double)), this, SLOT(OnAbsValueChanged(double)));
  connect(m_Controls->sbRelValue, SIGNAL(valueChanged(double)), this, SLOT(OnRelValueChanged(double)));
  connect(m_Controls->doseSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderChanged(int)));
  connect(m_Controls->checkVisibleIso, SIGNAL(clicked(bool)), this, SLOT(OnVisibleClicked(bool)));
  connect(m_Controls->colorBtn, SIGNAL(colorChanged(QColor)), this, SLOT(OnColorChanged(QColor)));
}

QmitkFreeIsoDoseLevelWidget::~QmitkFreeIsoDoseLevelWidget()
{
}

mitk::DoseValueAbs
  QmitkFreeIsoDoseLevelWidget::
  getReferenceDose() const
{
  return m_ReferenceDose;
};

mitk::IsoDoseLevel*
  QmitkFreeIsoDoseLevelWidget::
  getIsoDoseLevel() const
{
  return m_IsoDoseLevel;
};

void QmitkFreeIsoDoseLevelWidget::
  setReferenceDose(double newReferenceDose)
{
  if (newReferenceDose != m_ReferenceDose)
  {
    m_ReferenceDose = newReferenceDose;
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

    m_IsoDoseLevel = level;
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
    updateValue(newValue/m_ReferenceDose);
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
  m_IsoDoseLevel->SetVisibleIsoLine(checked);
  emit VisualizationStyleChanged(m_IsoDoseLevel);
};

void QmitkFreeIsoDoseLevelWidget::
  OnColorChanged(QColor color)
{
  mitk::IsoDoseLevel::ColorType doseColor;
  doseColor.SetRed(color.redF());
  doseColor.SetGreen(color.greenF());
  doseColor.SetBlue(color.blueF());
  m_IsoDoseLevel->SetColor(doseColor);
  emit ColorChanged(m_IsoDoseLevel);
};

void QmitkFreeIsoDoseLevelWidget::
  updateValue(mitk::DoseValueRel newDose)
{
  m_InternalUpdate = true;

  mitk::DoseValueRel oldValue = m_IsoDoseLevel->GetDoseValue();
  m_IsoDoseLevel->SetDoseValue(newDose);
  m_Controls->sbAbsValue->setValue(newDose*m_ReferenceDose);
  m_Controls->sbRelValue->setValue(newDose*100);
  m_Controls->doseSlider->setValue(newDose*100);

  m_InternalUpdate = false;

  emit ValueChanged(m_IsoDoseLevel,oldValue);
};

void QmitkFreeIsoDoseLevelWidget::
  update()
{
  updateValue(m_IsoDoseLevel->GetDoseValue());

  m_Controls->checkVisibleIso->setChecked(m_IsoDoseLevel->GetVisibleIsoLine());

  QColor color;
  color.setRgbF(m_IsoDoseLevel->GetColor().GetRed(),m_IsoDoseLevel->GetColor().GetGreen(),m_IsoDoseLevel->GetColor().GetBlue());
  m_Controls->colorBtn->setColor(color);
};

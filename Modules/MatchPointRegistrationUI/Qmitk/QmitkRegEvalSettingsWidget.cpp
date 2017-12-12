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

#include "QmitkRegEvalSettingsWidget.h"

#include "mitkMatchPointPropertyTags.h"
#include "mitkRegEvalStyleProperty.h"
#include "mitkRegEvalWipeStyleProperty.h"
#include "mitkRegEvaluationObject.h"
#include "mitkRegVisPropertyTags.h"
#include <mitkNodePredicateProperty.h>

void QmitkRegEvalSettingsWidget::SetNode(mitk::DataNode *node)
{
  if (this->m_selectedEvalNode.GetPointer() != node)
  {
    this->m_selectedEvalNode = node;
    ConfigureControls();
  }
};

QmitkRegEvalSettingsWidget::QmitkRegEvalSettingsWidget(QWidget *parent)
  : QWidget(parent), m_internalBlendUpdate(false), m_internalUpdate(false)
{
  this->setupUi(this);

  mitk::RegEvalStyleProperty::Pointer sampleProp = mitk::RegEvalStyleProperty::New();

  for (unsigned int pos = 0; pos < sampleProp->Size(); ++pos)
  {
    this->comboStyle->insertItem(pos, QString::fromStdString(sampleProp->GetEnumString(pos)));
  }

  connect(comboStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboStyleChanged(int)));

  connect(pbBlend50, SIGNAL(clicked()), this, SLOT(OnBlend50Pushed()));
  connect(pbBlendTarget, SIGNAL(clicked()), this, SLOT(OnBlendTargetPushed()));
  connect(pbBlendMoving, SIGNAL(clicked()), this, SLOT(OnBlendMovingPushed()));
  connect(pbBlendToggle, SIGNAL(clicked()), this, SLOT(OnBlendTogglePushed()));
  connect(slideBlend, SIGNAL(valueChanged(int)), this, SLOT(OnSlideBlendChanged(int)));
  connect(sbBlend, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBlendChanged(int)));

  connect(sbChecker, SIGNAL(valueChanged(int)), this, SLOT(OnSpinCheckerChanged(int)));

  connect(radioWipeCross, SIGNAL(toggled(bool)), this, SLOT(OnWipeStyleChanged()));
  connect(radioWipeH, SIGNAL(toggled(bool)), this, SLOT(OnWipeStyleChanged()));
  connect(radioWipeV, SIGNAL(toggled(bool)), this, SLOT(OnWipeStyleChanged()));

  connect(radioTargetContour, SIGNAL(toggled(bool)), this, SLOT(OnContourStyleChanged()));

  this->ConfigureControls();
}

void QmitkRegEvalSettingsWidget::ConfigureControls()
{
  m_internalUpdate = true;
  this->comboStyle->setEnabled(this->m_selectedEvalNode.IsNotNull());

  if (this->m_selectedEvalNode.IsNotNull())
  {
    mitk::RegEvalStyleProperty *evalProp = nullptr;

    if (this->m_selectedEvalNode->GetProperty(evalProp, mitk::nodeProp_RegEvalStyle))
    {
      OnComboStyleChanged(evalProp->GetValueAsId());
      this->comboStyle->setCurrentIndex(evalProp->GetValueAsId());
    }

    int factor = 50;
    this->m_selectedEvalNode->GetIntProperty(mitk::nodeProp_RegEvalBlendFactor, factor);
    this->sbBlend->setValue(factor);

    int count = 3;
    this->m_selectedEvalNode->GetIntProperty(mitk::nodeProp_RegEvalCheckerCount, count);
    this->sbChecker->setValue(count);

    bool targetContour = true;
    this->m_selectedEvalNode->GetBoolProperty(mitk::nodeProp_RegEvalTargetContour, targetContour);
    this->radioTargetContour->setChecked(targetContour);
  }
  else
  {
    this->groupBlend->setVisible(false);
    this->groupCheck->setVisible(false);
    this->groupWipe->setVisible(false);
    this->groupContour->setVisible(false);
  }
  m_internalUpdate = false;
}

void QmitkRegEvalSettingsWidget::OnComboStyleChanged(int index)
{
  groupBlend->setVisible(index == 0);
  groupCheck->setVisible(index == 2);
  groupWipe->setVisible(index == 3);
  groupContour->setVisible(index == 5);

  if (m_selectedEvalNode.IsNotNull())
  {
    m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalStyle, mitk::RegEvalStyleProperty::New(index));
    if (!m_internalUpdate)
    {
      emit SettingsChanged(m_selectedEvalNode.GetPointer());
    }
  }
};

void QmitkRegEvalSettingsWidget::OnBlend50Pushed()
{
  sbBlend->setValue(50);
};

void QmitkRegEvalSettingsWidget::OnBlendTargetPushed()
{
  sbBlend->setValue(0);
};

void QmitkRegEvalSettingsWidget::OnBlendMovingPushed()
{
  sbBlend->setValue(100);
};

void QmitkRegEvalSettingsWidget::OnBlendTogglePushed()
{
  sbBlend->setValue(100 - sbBlend->value());
};

void QmitkRegEvalSettingsWidget::OnSlideBlendChanged(int factor)
{
  m_internalBlendUpdate = true;
  sbBlend->setValue(factor);
  m_internalBlendUpdate = false;
};

void QmitkRegEvalSettingsWidget::OnSpinBlendChanged(int factor)
{
  if (m_selectedEvalNode.IsNotNull())
  {
    m_selectedEvalNode->SetIntProperty(mitk::nodeProp_RegEvalBlendFactor, factor);

    if (!m_internalBlendUpdate)
    {
      this->slideBlend->setValue(factor);
    }
    if (!m_internalUpdate)
    {
      emit SettingsChanged(m_selectedEvalNode.GetPointer());
    }
  }
};

void QmitkRegEvalSettingsWidget::OnSpinCheckerChanged(int count)
{
  if (m_selectedEvalNode.IsNotNull())
  {
    m_selectedEvalNode->SetIntProperty(mitk::nodeProp_RegEvalCheckerCount, count);
    if (!m_internalUpdate)
    {
      emit SettingsChanged(m_selectedEvalNode.GetPointer());
    }
  }
};

void QmitkRegEvalSettingsWidget::OnWipeStyleChanged()
{
  if (m_selectedEvalNode.IsNotNull())
  {
    if (this->radioWipeCross->isChecked())
    {
      m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalWipeStyle, mitk::RegEvalWipeStyleProperty::New(0));
    }
    else if (this->radioWipeH->isChecked())
    {
      m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalWipeStyle, mitk::RegEvalWipeStyleProperty::New(1));
    }
    else
    {
      m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalWipeStyle, mitk::RegEvalWipeStyleProperty::New(2));
    }

    if (!m_internalUpdate)
    {
      emit SettingsChanged(m_selectedEvalNode.GetPointer());
    }
  }
};

void QmitkRegEvalSettingsWidget::OnContourStyleChanged()
{
  if (m_selectedEvalNode.IsNotNull())
  {
    m_selectedEvalNode->SetBoolProperty(mitk::nodeProp_RegEvalTargetContour, radioTargetContour->isChecked());
    if (!m_internalUpdate)
    {
      emit SettingsChanged(m_selectedEvalNode.GetPointer());
    }
  }
};

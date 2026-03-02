/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRegEvalSettingsWidget.h"

#include "mitkMatchPointPropertyTags.h"
#include "mitkRegEvalStyleProperty.h"
#include "mitkRegEvalWipeStyleProperty.h"
#include "mitkRegEvaluationObject.h"
#include "mitkRegVisPropertyTags.h"
#include <mitkNodePredicateProperty.h>

#include <ui_QmitkRegEvalSettingsWidget.h>

void QmitkRegEvalSettingsWidget::SetNode(mitk::DataNode *node)
{
  if (m_selectedEvalNode.GetPointer() != node)
  {
    m_selectedEvalNode = node;
    ConfigureControls();
  }
};

QmitkRegEvalSettingsWidget::QmitkRegEvalSettingsWidget(QWidget *parent)
  : QWidget(parent), m_Controls(std::make_unique<Ui::QmitkRegEvalSettingsWidget>()), m_internalBlendUpdate(false), m_internalUpdate(false)
{
  m_Controls->setupUi(this);

  mitk::RegEvalStyleProperty::Pointer sampleProp = mitk::RegEvalStyleProperty::New();

  for (unsigned int pos = 0; pos < sampleProp->Size(); ++pos)
  {
    m_Controls->comboStyle->insertItem(pos, QString::fromStdString(sampleProp->GetEnumString(pos)));
  }

  connect(m_Controls->comboStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboStyleChanged(int)));

  connect(m_Controls->pbBlend50, SIGNAL(clicked()), this, SLOT(OnBlend50Pushed()));
  connect(m_Controls->pbBlendTarget, SIGNAL(clicked()), this, SLOT(OnBlendTargetPushed()));
  connect(m_Controls->pbBlendMoving, SIGNAL(clicked()), this, SLOT(OnBlendMovingPushed()));
  connect(m_Controls->pbBlendToggle, SIGNAL(clicked()), this, SLOT(OnBlendTogglePushed()));
  connect(m_Controls->slideBlend, SIGNAL(valueChanged(int)), this, SLOT(OnSlideBlendChanged(int)));
  connect(m_Controls->sbBlend, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBlendChanged(int)));

  connect(m_Controls->sbChecker, SIGNAL(valueChanged(int)), this, SLOT(OnSpinCheckerChanged(int)));

  connect(m_Controls->radioWipeCross, SIGNAL(toggled(bool)), this, SLOT(OnWipeStyleChanged()));
  connect(m_Controls->radioWipeH, SIGNAL(toggled(bool)), this, SLOT(OnWipeStyleChanged()));
  connect(m_Controls->radioWipeV, SIGNAL(toggled(bool)), this, SLOT(OnWipeStyleChanged()));

  connect(m_Controls->radioTargetContour, SIGNAL(toggled(bool)), this, SLOT(OnContourStyleChanged()));

  this->ConfigureControls();
}

QmitkRegEvalSettingsWidget::~QmitkRegEvalSettingsWidget()
{
}

void QmitkRegEvalSettingsWidget::ConfigureControls()
{
  m_internalUpdate = true;
  m_Controls->comboStyle->setEnabled(m_selectedEvalNode.IsNotNull());

  if (m_selectedEvalNode.IsNotNull())
  {
    mitk::RegEvalStyleProperty *evalProp = nullptr;

    if (m_selectedEvalNode->GetProperty(evalProp, mitk::nodeProp_RegEvalStyle))
    {
      OnComboStyleChanged(evalProp->GetValueAsId());
      m_Controls->comboStyle->setCurrentIndex(evalProp->GetValueAsId());
    }

    int factor = 50;
    m_selectedEvalNode->GetIntProperty(mitk::nodeProp_RegEvalBlendFactor, factor);
    m_Controls->sbBlend->setValue(factor);

    int count = 3;
    m_selectedEvalNode->GetIntProperty(mitk::nodeProp_RegEvalCheckerCount, count);
    m_Controls->sbChecker->setValue(count);

    bool targetContour = true;
    m_selectedEvalNode->GetBoolProperty(mitk::nodeProp_RegEvalTargetContour, targetContour);
    m_Controls->radioTargetContour->setChecked(targetContour);
  }
  else
  {
    m_Controls->groupBlend->setVisible(false);
    m_Controls->groupCheck->setVisible(false);
    m_Controls->groupWipe->setVisible(false);
    m_Controls->groupContour->setVisible(false);
  }
  m_internalUpdate = false;
}

void QmitkRegEvalSettingsWidget::OnComboStyleChanged(int index)
{
  m_Controls->groupBlend->setVisible(index == 0);
  m_Controls->groupCheck->setVisible(index == 2);
  m_Controls->groupWipe->setVisible(index == 3);
  m_Controls->groupContour->setVisible(index == 5);

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
  m_Controls->sbBlend->setValue(50);
};

void QmitkRegEvalSettingsWidget::OnBlendTargetPushed()
{
  m_Controls->sbBlend->setValue(0);
};

void QmitkRegEvalSettingsWidget::OnBlendMovingPushed()
{
  m_Controls->sbBlend->setValue(100);
};

void QmitkRegEvalSettingsWidget::OnBlendTogglePushed()
{
  m_Controls->sbBlend->setValue(100 - m_Controls->sbBlend->value());
};

void QmitkRegEvalSettingsWidget::OnSlideBlendChanged(int factor)
{
  m_internalBlendUpdate = true;
  m_Controls->sbBlend->setValue(factor);
  m_internalBlendUpdate = false;
};

void QmitkRegEvalSettingsWidget::OnSpinBlendChanged(int factor)
{
  if (m_selectedEvalNode.IsNotNull())
  {
    m_selectedEvalNode->SetIntProperty(mitk::nodeProp_RegEvalBlendFactor, factor);

    if (!m_internalBlendUpdate)
    {
      m_Controls->slideBlend->setValue(factor);
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
    if (m_Controls->radioWipeCross->isChecked())
    {
      m_selectedEvalNode->SetProperty(mitk::nodeProp_RegEvalWipeStyle, mitk::RegEvalWipeStyleProperty::New(0));
    }
    else if (m_Controls->radioWipeH->isChecked())
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
    m_selectedEvalNode->SetBoolProperty(mitk::nodeProp_RegEvalTargetContour, m_Controls->radioTargetContour->isChecked());
    if (!m_internalUpdate)
    {
      emit SettingsChanged(m_selectedEvalNode.GetPointer());
    }
  }
};

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkBoolPropertyWidget.h"

#include <mitkPropertyObserver.h>

class _BoolPropertyWidgetImpl : public mitk::PropertyEditor
{
public:
  _BoolPropertyWidgetImpl(mitk::BoolProperty *property, QCheckBox *checkBox)
    : PropertyEditor(property), m_BoolProperty(property), m_CheckBox(checkBox)
  {
  }

  void PropertyChanged() override
  {
    if (m_Property)
      m_CheckBox->setChecked(m_BoolProperty->GetValue());
  }

  void PropertyRemoved() override
  {
    m_Property = nullptr;
    m_BoolProperty = nullptr;
    // display "no certain value"
    m_CheckBox->blockSignals(true);
    m_CheckBox->setTristate(true);
    m_CheckBox->setCheckState(Qt::PartiallyChecked);
    m_CheckBox->setEnabled(false);
    m_CheckBox->blockSignals(false);
  }

  void ValueChanged(bool value)
  {
    this->BeginModifyProperty(); // deregister from events
    m_BoolProperty->SetValue(value);
    this->EndModifyProperty(); // again register for events
  }

protected:
  mitk::BoolProperty *m_BoolProperty;
  QCheckBox *m_CheckBox;
};

QmitkBoolPropertyWidget::QmitkBoolPropertyWidget(QWidget *parent) : QCheckBox(parent), m_PropEditorImpl(nullptr)
{
  setEnabled(false);

  connect(this, SIGNAL(toggled(bool)), this, SLOT(onToggle(bool)));
}

QmitkBoolPropertyWidget::QmitkBoolPropertyWidget(const QString &text, QWidget *parent)
  : QCheckBox(text, parent), m_PropEditorImpl(nullptr)
{
  setEnabled(false);

  connect(this, SIGNAL(toggled(bool)), this, SLOT(onToggle(bool)));
}

QmitkBoolPropertyWidget::~QmitkBoolPropertyWidget()
{
  delete m_PropEditorImpl;
}

void QmitkBoolPropertyWidget::SetProperty(mitk::BoolProperty *property)
{
  if (m_PropEditorImpl)
  {
    delete m_PropEditorImpl;
    m_PropEditorImpl = nullptr;
  }

  if (!property)
  {
    setTristate(true);
    setCheckState(Qt::PartiallyChecked);
    setEnabled(false);
    return;
  }

  setEnabled(true);
  m_PropEditorImpl = new _BoolPropertyWidgetImpl(property, this);
  m_PropEditorImpl->PropertyChanged();
}

void QmitkBoolPropertyWidget::onToggle(bool on)
{
  if (m_PropEditorImpl)
  {
    m_PropEditorImpl->ValueChanged(on);
  }
}

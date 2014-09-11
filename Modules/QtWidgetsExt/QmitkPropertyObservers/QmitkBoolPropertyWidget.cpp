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

#include "QmitkBoolPropertyWidget.h"

#include <mitkPropertyObserver.h>

class _BoolPropertyWidgetImpl : public mitk::PropertyEditor
{
public:

  _BoolPropertyWidgetImpl(mitk::BoolProperty* property, QCheckBox* checkBox)
    : PropertyEditor(property), m_BoolProperty(property), m_CheckBox(checkBox)
  {
  }

  virtual void PropertyChanged()
  {
    if ( m_Property )
      m_CheckBox->setChecked( m_BoolProperty->GetValue() );
  }

  virtual void PropertyRemoved()
  {
    m_Property = NULL;
    m_BoolProperty = NULL;
    // display "no certain value"
    m_CheckBox->blockSignals(true);
    m_CheckBox->setTristate(true);
    m_CheckBox->setCheckState(Qt::PartiallyChecked);
    m_CheckBox->setEnabled(false);
    m_CheckBox->blockSignals(false);
  }

  void ValueChanged(bool value)
  {
    this->BeginModifyProperty();  // deregister from events
    m_BoolProperty->SetValue(value);
    this->EndModifyProperty();  // again register for events
  }

protected:

  mitk::BoolProperty* m_BoolProperty;
  QCheckBox* m_CheckBox;

};

QmitkBoolPropertyWidget::QmitkBoolPropertyWidget(QWidget* parent )
  : QCheckBox(parent), m_PropEditorImpl(0)
{
  setEnabled(false);

  connect(this, SIGNAL(toggled(bool)), this, SLOT(onToggle(bool)));
}

QmitkBoolPropertyWidget::QmitkBoolPropertyWidget(const QString& text, QWidget* parent )
: QCheckBox(text, parent ), m_PropEditorImpl(0)
{
  setEnabled( false );

  connect(this, SIGNAL(toggled(bool)), this, SLOT(onToggle(bool)));
}

QmitkBoolPropertyWidget::~QmitkBoolPropertyWidget()
{
  delete m_PropEditorImpl;
}

void QmitkBoolPropertyWidget::SetProperty(mitk::BoolProperty* property)
{
  if (m_PropEditorImpl)
  {
    delete m_PropEditorImpl;
    m_PropEditorImpl = 0;
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


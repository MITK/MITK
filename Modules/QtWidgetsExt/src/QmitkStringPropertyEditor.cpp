/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkStringPropertyEditor.h"

QmitkStringPropertyEditor::QmitkStringPropertyEditor(mitk::StringProperty *property, QWidget *parent)
  : QLineEdit(parent), PropertyEditor(property), m_StringProperty(property)
{
  PropertyChanged();
  connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(onTextChanged(const QString &)));
}

QmitkStringPropertyEditor::~QmitkStringPropertyEditor()
{
}

void QmitkStringPropertyEditor::PropertyChanged()
{
  if (m_Property)
    setText(m_StringProperty->GetValue());
}

void QmitkStringPropertyEditor::PropertyRemoved()
{
  m_Property = nullptr;
  m_StringProperty = nullptr;
  setText("n/a");
}

void QmitkStringPropertyEditor::onTextChanged(const QString &text)
{
  if (m_StringProperty == nullptr)
    return;

  BeginModifyProperty(); // deregister from events

  m_StringProperty->SetValue(text.toStdString());

  EndModifyProperty(); // again register for events
}

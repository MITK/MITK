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
#include "QmitkStringPropertyEditor.h"

QmitkStringPropertyEditor::QmitkStringPropertyEditor( mitk::StringProperty* property, QWidget* parent )
: QLineEdit( parent ),
  PropertyEditor( property ),
  m_StringProperty(property)
{
  PropertyChanged();
  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
}

QmitkStringPropertyEditor::~QmitkStringPropertyEditor()
{
}

void QmitkStringPropertyEditor::PropertyChanged()
{
  if ( m_Property )
    setText( m_StringProperty->GetValue() );
}

void QmitkStringPropertyEditor::PropertyRemoved()
{
  m_Property = NULL;
  m_StringProperty = NULL;
  setText("n/a");
}

void QmitkStringPropertyEditor::onTextChanged(const QString& text)
{
  if( m_StringProperty == 0 )
    return;

  BeginModifyProperty();  // deregister from events

  m_StringProperty->SetValue(text.toStdString());

  EndModifyProperty();  // again register for events
}


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

#include "QmitkXnatObjectEditorInput.h"

#include <QIcon>

QmitkXnatObjectEditorInput::QmitkXnatObjectEditorInput(ctkXnatObject* object)
  : m_Object(object)
{
}

QmitkXnatObjectEditorInput::~QmitkXnatObjectEditorInput()
{
}

ctkXnatObject* QmitkXnatObjectEditorInput::GetXnatObject() const
{
  return m_Object;
}

bool QmitkXnatObjectEditorInput::Exists() const
{
  return m_Object->exists();
}

QString QmitkXnatObjectEditorInput::GetName() const
{
  return m_Object->id();
}

QString QmitkXnatObjectEditorInput::GetToolTipText() const
{
  return m_Object->description();
}

bool QmitkXnatObjectEditorInput::operator==(const berry::Object* o) const
{
  if ( const QmitkXnatObjectEditorInput* other = dynamic_cast<const QmitkXnatObjectEditorInput*>(o) )
  {
    if ( other->GetXnatObject()->parent() )
    {
      return (other->GetName() == this->GetName()) &&
        (other->GetXnatObject()->parent()->id() == this->GetXnatObject()->parent()->id());
    }
    else
    {
      return (other->GetName() == this->GetName());
    }
  }
  return false;
}


QIcon QmitkXnatObjectEditorInput::GetIcon() const
{
  return QIcon();
}

const berry::IPersistableElement* QmitkXnatObjectEditorInput::GetPersistable() const
{
  return nullptr;
}

berry::Object* QmitkXnatObjectEditorInput::GetAdapter(const QString& /*adapterType*/) const
{
  return nullptr;
}

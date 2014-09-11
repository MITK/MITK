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

std::string QmitkXnatObjectEditorInput::GetName() const
{
  return m_Object->id().toStdString();
}

std::string QmitkXnatObjectEditorInput::GetToolTipText() const
{
  return m_Object->description().toStdString();
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

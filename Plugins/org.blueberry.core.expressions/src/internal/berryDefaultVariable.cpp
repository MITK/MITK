/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryDefaultVariable.h"

#include <Poco/Bugcheck.h>

namespace berry {

DefaultVariable::DefaultVariable(IEvaluationContext* parent,
                                 const Object::ConstPointer& defaultVariable)
  : fDefaultVariable(defaultVariable)
  , fParent(parent)
{
  Q_ASSERT(parent);

  while (dynamic_cast<const DefaultVariable*>(parent))
  {
    parent = parent->GetParent();
  }
  fManagedPool = parent;
  fDefaultVariable = defaultVariable;
}

IEvaluationContext* DefaultVariable::GetParent() const
{
  return fParent;
}

IEvaluationContext* DefaultVariable::GetRoot() const
{
  return fParent->GetRoot();
}

Object::ConstPointer DefaultVariable::GetDefaultVariable() const
{
  return fDefaultVariable;
}

void DefaultVariable::SetAllowPluginActivation(bool value)
{
  fParent->SetAllowPluginActivation(value);
}

bool DefaultVariable::GetAllowPluginActivation() const
{
  return fParent->GetAllowPluginActivation();
}

void DefaultVariable::AddVariable(const QString& name, const Object::ConstPointer& value)
{
  fManagedPool->AddVariable(name, value);
}

Object::ConstPointer DefaultVariable::RemoveVariable(const QString &name)
{
  return fManagedPool->RemoveVariable(name);
}

Object::ConstPointer DefaultVariable::GetVariable(const QString &name) const
{
  return fManagedPool->GetVariable(name);
}

Object::ConstPointer DefaultVariable::ResolveVariable(const QString &name,
                                                 const QList<Object::Pointer>& args) const
{
  return fManagedPool->ResolveVariable(name, args);
}


}

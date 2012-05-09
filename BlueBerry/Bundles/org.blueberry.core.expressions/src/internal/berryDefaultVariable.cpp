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
                                 const Object::Pointer& defaultVariable)
{
  poco_check_ptr(parent);

  fParent= parent;

  while (dynamic_cast<DefaultVariable*>(parent))
  {
    parent = parent->GetParent();
  }
  fManagedPool= parent;
  fDefaultVariable= defaultVariable;
}

IEvaluationContext* DefaultVariable::GetParent() const
{
  return fParent;
}

IEvaluationContext* DefaultVariable::GetRoot()
{
  return fParent->GetRoot();
}

Object::Pointer DefaultVariable::GetDefaultVariable() const
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

void DefaultVariable::AddVariable(const QString& name, const Object::Pointer& value)
{
  fManagedPool->AddVariable(name, value);
}

Object::Pointer DefaultVariable::RemoveVariable(const QString &name)
{
  return fManagedPool->RemoveVariable(name);
}

Object::Pointer DefaultVariable::GetVariable(const QString &name) const
{
  return fManagedPool->GetVariable(name);
}

Object::Pointer DefaultVariable::ResolveVariable(const QString &name,
                                                 const QList<Object::Pointer>& args)
{
  return fManagedPool->ResolveVariable(name, args);
}


}

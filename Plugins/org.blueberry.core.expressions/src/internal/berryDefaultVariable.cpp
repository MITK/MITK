/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

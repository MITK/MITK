/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryEvaluationContext.h"

namespace berry {

EvaluationContext::EvaluationContext(IEvaluationContext* parent,
                                     const Object::ConstPointer& defaultVariable)
  : fParent(parent), fDefaultVariable(defaultVariable), fAllowPluginActivation(-1)
{
  poco_assert(defaultVariable != 0);
}

EvaluationContext::EvaluationContext(IEvaluationContext* parent,
                                     const Object::ConstPointer& defaultVariable,
                                     const std::vector<IVariableResolver*>& resolvers)
  : fParent(parent), fDefaultVariable(defaultVariable),
    fVariableResolvers(resolvers), fAllowPluginActivation(-1)
{
  poco_assert(defaultVariable != 0);
  poco_assert(resolvers.size() != 0);
}

IEvaluationContext* EvaluationContext::GetParent() const
{
  return fParent;
}

IEvaluationContext* EvaluationContext::GetRoot() const
{
  if (fParent == nullptr)
    return const_cast<EvaluationContext*>(this);
  return fParent->GetRoot();
}

Object::ConstPointer EvaluationContext::GetDefaultVariable() const
{
  return fDefaultVariable;
}

void EvaluationContext::SetAllowPluginActivation(bool value)
{
  fAllowPluginActivation= value ? 1 : 0;
}

bool EvaluationContext::GetAllowPluginActivation() const
{
  if (fAllowPluginActivation < 0)
  {
    if (fParent)
    {
      return fParent->GetAllowPluginActivation();
    }
    return false;
  }
  return fAllowPluginActivation;
}

void
EvaluationContext::AddVariable(const QString &name, const Object::ConstPointer& value)
{
  poco_assert(name.size() != 0);
  poco_assert(value.IsNotNull());

  fVariables[name] = value;
}

Object::ConstPointer
EvaluationContext::RemoveVariable(const QString &name)
{
  poco_assert(name.size() != 0);

  Object::ConstPointer elem(fVariables[name]);
  fVariables.remove(name);
  return elem;
}

Object::ConstPointer
EvaluationContext::GetVariable(const QString& name) const
{
  poco_assert(name.size() != 0);

  Object::ConstPointer result;

  QHash<QString, Object::ConstPointer>::const_iterator iter(fVariables.find(name));
  if (iter != fVariables.end())
  {
    result = iter.value();
  }

  if (!result.IsNull())
    return result;

  if (fParent != nullptr)
    return fParent->GetVariable(name);

  return result;
}

Object::ConstPointer
EvaluationContext::ResolveVariable(const QString &name, const QList<Object::Pointer>& args) const
{
  if (fVariableResolvers.size() > 0) {
    for (unsigned int i= 0; i < fVariableResolvers.size(); ++i) {
      IVariableResolver* resolver = fVariableResolvers[i];
      Object::Pointer variable(resolver->Resolve(name, args));
      if (!variable.IsNull())
        return variable;
    }
  }

  if (fParent != nullptr)
    return fParent->ResolveVariable(name, args);

  return Object::Pointer();
}

}

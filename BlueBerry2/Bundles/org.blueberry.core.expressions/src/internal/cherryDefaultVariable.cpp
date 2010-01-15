/*=========================================================================

Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cherryDefaultVariable.h"

#include <Poco/Bugcheck.h>

namespace cherry {

DefaultVariable::DefaultVariable(IEvaluationContext::Pointer parent,
    Object::Pointer defaultVariable)
{
  poco_check_ptr(parent);

  fParent= parent;

  while (parent.Cast<DefaultVariable>())
  {
    parent = parent->GetParent();
  }
  fManagedPool= parent;
  fDefaultVariable= defaultVariable;
}

IEvaluationContext::Pointer DefaultVariable::GetParent() const
{
  return fParent;
}

IEvaluationContext::Pointer DefaultVariable::GetRoot()
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

void DefaultVariable::AddVariable(const std::string& name, Object::Pointer value)
{
  fManagedPool->AddVariable(name, value);
}

Object::Pointer DefaultVariable::RemoveVariable(const std::string& name)
{
  return fManagedPool->RemoveVariable(name);
}

Object::Pointer DefaultVariable::GetVariable(const std::string& name) const
{
  return fManagedPool->GetVariable(name);
}

Object::Pointer DefaultVariable::ResolveVariable(const std::string& name,
    std::vector<Object::Pointer>& args)
{
  return fManagedPool->ResolveVariable(name, args);
}


}

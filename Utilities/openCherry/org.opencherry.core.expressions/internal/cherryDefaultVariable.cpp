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

namespace cherry {

DefaultVariable::DefaultVariable(IEvaluationContext* parent,
    ExpressionVariable::Pointer defaultVariable)
{
  if (parent == 0)
    throw Poco::AssertionViolationException("parent evaluation context must not be 0");

  fParent= parent;

  while (dynamic_cast<DefaultVariable*>(parent))
  {
    parent = parent->GetParent();
  }
  fManagedPool= parent;
  fDefaultVariable= defaultVariable;
}

IEvaluationContext* DefaultVariable::GetParent()
{
  return fParent;
}

IEvaluationContext* DefaultVariable::GetRoot()
{
  return fParent->GetRoot();
}

ExpressionVariable::Pointer DefaultVariable::GetDefaultVariable()
{
  return fDefaultVariable;
}

void DefaultVariable::SetAllowPluginActivation(bool value)
{
  fParent->SetAllowPluginActivation(value);
}

bool DefaultVariable::GetAllowPluginActivation()
{
  return fParent->GetAllowPluginActivation();
}

void DefaultVariable::AddVariable(const std::string& name, ExpressionVariable::Pointer value)
{
  fManagedPool->AddVariable(name, value);
}

ExpressionVariable::Pointer DefaultVariable::RemoveVariable(const std::string& name)
{
  return fManagedPool->RemoveVariable(name);
}

ExpressionVariable::Pointer DefaultVariable::GetVariable(const std::string& name)
{
  return fManagedPool->GetVariable(name);
}

ExpressionVariable::Pointer DefaultVariable::ResolveVariable(const std::string& name,
    std::vector<ExpressionVariable::Pointer>& args)
{
  return fManagedPool->ResolveVariable(name, args);
}


}

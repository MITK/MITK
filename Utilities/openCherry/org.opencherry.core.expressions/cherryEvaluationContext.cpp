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

#include "cherryEvaluationContext.h"

namespace cherry {

EvaluationContext::EvaluationContext(IEvaluationContext* parent, 
                                     ExpressionVariable::Pointer defaultVariable) 
{
  poco_assert(defaultVariable.IsNotNull());

  fParent = parent;
  fDefaultVariable = defaultVariable;
  fAllowPluginActivation = parent->GetAllowPluginActivation();
}

EvaluationContext::EvaluationContext(IEvaluationContext* parent, 
    ExpressionVariable::Pointer defaultVariable, 
    std::vector<IVariableResolver*> resolvers) 
{
  poco_assert(defaultVariable.IsNotNull());
    
  poco_assert(resolvers.size() != 0);
  
  fParent= parent;
  fDefaultVariable= defaultVariable;
  fVariableResolvers= resolvers;
  fAllowPluginActivation = parent->GetAllowPluginActivation();
}

IEvaluationContext*
EvaluationContext::GetParent() 
{
  return fParent;
}

IEvaluationContext*
EvaluationContext::GetRoot() 
{
  if (fParent == 0)
    return this;
  return fParent->GetRoot();
}

ExpressionVariable::Pointer 
EvaluationContext::GetDefaultVariable() 
{
  return fDefaultVariable;
}

void 
EvaluationContext::SetAllowPluginActivation(bool value) 
{
  fAllowPluginActivation= value;
}

bool 
EvaluationContext::GetAllowPluginActivation() 
{
  return fAllowPluginActivation;
}

void 
EvaluationContext::AddVariable(const std::string& name, ExpressionVariable::Pointer value) 
{
  poco_assert(name.size() != 0);
  poco_assert(value.IsNotNull());
  
  fVariables[name] = value;
}

ExpressionVariable::Pointer 
EvaluationContext::RemoveVariable(const std::string& name) 
{
  poco_assert(name.size() != 0);
  
  ExpressionVariable::Pointer elem(fVariables[name]);
  fVariables.erase(name);
  return elem;
}

ExpressionVariable::Pointer 
EvaluationContext::GetVariable(const std::string& name) 
{
  poco_assert(name.size() != 0);
  
  ExpressionVariable::Pointer result;
  
  std::map<std::string, ExpressionVariable::Pointer>::iterator iter(fVariables.find(name));
  if (iter != fVariables.end()) {
    result = iter->second;
  }
  
  if (!result.IsNull())
    return result;
  
  if (fParent != 0)
    return fParent->GetVariable(name);
  
  return result;
}

ExpressionVariable::Pointer 
EvaluationContext::ResolveVariable(const std::string& name, std::vector<ExpressionVariable::Pointer>& args) 
{
  if (fVariableResolvers.size() > 0) {
    for (unsigned int i= 0; i < fVariableResolvers.size(); ++i) {
      IVariableResolver* resolver = fVariableResolvers[i];
      ExpressionVariable::Pointer variable(resolver->Resolve(name, args));
      if (!variable.IsNull())
        return variable;
    }
  }
  
  if (fParent != 0)
    return fParent->ResolveVariable(name, args);
  
  return ExpressionVariable::Pointer();
}

}

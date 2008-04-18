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

#ifndef CHERRYIEVALUATIONCONTEXT_H_
#define CHERRYIEVALUATIONCONTEXT_H_

#include <string>
#include <vector>

#include <org.opencherry.osgi/cherryMacros.h>
#include "org.opencherry.core.runtime/cherryExpressionVariables.h"

namespace cherry {

/**
 * An evaluation context is used to manage a set of objects needed during
 * XML expression evaluation. A context has a parent context, can manage
 * a set of named variables and has a default variable. The default variable 
 * is used during XML expression evaluation if no explicit variable is 
 * referenced.
 * <p>
 * This interface is not intended to be implemented by clients. Clients
 * are allowed to instantiate <code>EvaluationContext</code>. 
 * </p>
 * 
 * @since 3.0
 */
struct CHERRY_API IEvaluationContext : public Object {

  cherryClassMacro(IEvaluationContext);
  
  virtual ~IEvaluationContext() {}
  
  /**
   * Returns the parent context or <code>null</code> if 
   * this is the root of the evaluation context hierarchy.
   * 
   * @return the parent evaluation context or <code>null</code>
   */
  virtual IEvaluationContext* GetParent() = 0;
  
  /**
   * Returns the root evaluation context.
   * 
   * @return the root evaluation context
   */
  virtual IEvaluationContext* GetRoot() = 0;
  
  /**
   * Specifies whether this evaluation context allows activation
   * of plug-ins for testers used in the expression tree. To actual
   * trigger the plug-in loading this flag has to be set to <code>
   * true</code> and the actual test expression must have the
   * attribute <code>forcePluginActivation</code> set to <code>
   * true</code> as well.
   * 
   * @param value whether this evaluation context allows plug-in activation
   * @since 3.2
   */
  virtual void SetAllowPluginActivation(bool value) = 0;
  
  /**
   * Returns whether this evaluation context supports plug-in 
   * activation. If not set via {@link #setAllowPluginActivation(boolean)}
   * the parent value is returned. If no parent is set <code>false</code>
   * is returned.
   * 
   * @return whether plug-in activation is supported or not
   * @since 3.2
   */
  virtual bool GetAllowPluginActivation() = 0;
  
  /**
   * Returns the default variable.
   * 
   * @return the default variable or <code>null</code> if
   *  no default variable is managed.
   */
  virtual ExpressionVariable::Pointer GetDefaultVariable() = 0;
  
  /**
   * Adds a new named variable to this context. If a variable
   * with the name already exists the new one overrides the
   * existing one.
   * 
   * @param name the variable's name
   * @param value the variable's value
   */
  virtual void AddVariable(const std::string& name, ExpressionVariable::Pointer value) = 0;
  
  /**
   * Removes the variable managed under the given name
   * from this evaluation context.
   * 
   * @param name the variable's name
   * @return the currently stored value or <code>null</code> if
   *  the variable doesn't exist
   */
  virtual ExpressionVariable::Pointer RemoveVariable(const std::string& name) = 0;
  
  /**
   * Returns the variable managed under the given name.
   * 
   * @param name the variable's name
   * @return the variable's value or <code>null</code> if the content
   *  doesn't manage a variable with the given name 
   */
  virtual ExpressionVariable::Pointer GetVariable(const std::string& name) = 0;
  
  /**
   * Resolves a variable for the given name and arguments. This
   * method can be used to dynamically resolve variable such as
   * plug-in descriptors, resources, etc. The method is used
   * by the <code>resolve</code> expression.
   * 
   * @param name the variable to resolve
   * @param args an object array of arguments used to resolve the
   *  variable
   * @return the variable's value or <code>null</code> if no variable
   *  can be resolved for the given name and arguments
   * @exception CoreException if an errors occurs while resolving
   *  the variable
   */
  virtual ExpressionVariable::Pointer ResolveVariable(const std::string& name, std::vector<ExpressionVariable::Pointer>& args) = 0;
};

}  // namespace cherry

#endif /*CHERRYIEVALUATIONCONTEXT_H_*/

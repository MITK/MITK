/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef BERRYIEVALUATIONCONTEXT_H_
#define BERRYIEVALUATIONCONTEXT_H_

#include <string>
#include <vector>

#include <berryMacros.h>
#include <berryObject.h>

#include "berryExpressionsDll.h"

namespace berry {

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
struct BERRY_EXPRESSIONS IEvaluationContext : public Object {

  berryInterfaceMacro(IEvaluationContext, berry);

  virtual ~IEvaluationContext() {}

  /**
   * Returns the parent context or <code>null</code> if
   * this is the root of the evaluation context hierarchy.
   *
   * @return the parent evaluation context or <code>null</code>
   */
  virtual IEvaluationContext* GetParent() const = 0;

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
  virtual bool GetAllowPluginActivation() const = 0;

  /**
   * Returns the default variable.
   *
   * @return the default variable or <code>null</code> if
   *  no default variable is managed.
   */
  virtual Object::Pointer GetDefaultVariable() const = 0;

  /**
   * Adds a new named variable to this context. If a variable
   * with the name already exists the new one overrides the
   * existing one.
   *
   * @param name the variable's name
   * @param value the variable's value
   */
  virtual void AddVariable(const std::string& name, Object::Pointer value) = 0;

  /**
   * Removes the variable managed under the given name
   * from this evaluation context.
   *
   * @param name the variable's name
   * @return the currently stored value or <code>null</code> if
   *  the variable doesn't exist
   */
  virtual Object::Pointer RemoveVariable(const std::string& name) = 0;

  /**
   * Returns the variable managed under the given name.
   *
   * @param name the variable's name
   * @return the variable's value or <code>null</code> if the content
   *  doesn't manage a variable with the given name
   */
  virtual Object::Pointer GetVariable(const std::string& name) const = 0;

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
  virtual Object::Pointer ResolveVariable(const std::string& name, std::vector<Object::Pointer>& args) = 0;
};

}  // namespace berry

#endif /*BERRYIEVALUATIONCONTEXT_H_*/

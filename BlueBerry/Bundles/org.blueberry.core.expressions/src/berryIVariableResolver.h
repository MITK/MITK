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

#ifndef BERRYIVARIABLERESOLVER_H_
#define BERRYIVARIABLERESOLVER_H_

#include "berryExpressionsDll.h"

namespace berry {

/**
 * A variable resolver can be used to add additional variable resolving
 * strategies to an {@link EvaluationContext}.
 * 
 * @see org.blueberry.core.expressions.EvaluationContext#resolveVariable(String, Object[])
 * 
 * @since 3.0
 */
struct BERRY_EXPRESSIONS IVariableResolver {

	virtual ~IVariableResolver() {};
  
  /**
   * Resolves a variable for the given name and arguments. The
   * handler is allowed to return <code>null</code> to indicate
   * that it is not able to resolve the requested variable.
   * 
   * @param name the variable to resolve
   * @param args an object array of arguments used to resolve the
   *  variable
   * @return the variable's value or <code>null</code> if no variable
   *  could be resolved
   * @exception CoreException if an errors occurs while resolving
   *  the variable
   */
  virtual Object::Pointer Resolve(const std::string& name, std::vector<Object::Pointer> args) = 0;
};

}  // namespace berry

#endif /*BERRYIVARIABLERESOLVER_H_*/

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIVARIABLERESOLVER_H_
#define BERRYIVARIABLERESOLVER_H_

#include <org_blueberry_core_expressions_Export.h>

#include <QList>

#include <berryObject.h>

namespace berry {

/**
 * A variable resolver can be used to add additional variable resolving
 * strategies to an {@link EvaluationContext}.
 *
 * @see EvaluationContext#ResolveVariable(String, Object[])
 */
struct BERRY_EXPRESSIONS IVariableResolver {

  virtual ~IVariableResolver();

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
  virtual Object::Pointer Resolve(const QString& name, const QList<Object::Pointer>& args) = 0;
};

}  // namespace berry

#endif /*BERRYIVARIABLERESOLVER_H_*/

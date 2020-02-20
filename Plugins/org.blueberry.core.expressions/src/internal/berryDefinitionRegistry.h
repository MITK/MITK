/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYDEFINITIONREGISTRY_H_
#define BERRYDEFINITIONREGISTRY_H_

#include "berryExpression.h"

#include <QHash>

namespace berry {

struct IConfigurationElement;

/**
 * This manages the extension point that allows core expression reuse.
 */
class DefinitionRegistry { //implements IRegistryChangeListener {

private:

  QHash<QString, Expression::Pointer> cache;

  QHash<QString, Expression::Pointer>& GetCache();

public:
  DefinitionRegistry();

  /**
   * Get the expression with the id defined by an extension. This class will
   * cache the expressions when appropriate, so it's OK to always ask the
   * registry.
   *
   * @param id The unique ID of the expression definition
   * @return the expression
   * @throws CoreException If the expression cannot be found.
   */
  Expression::Pointer GetExpression(const QString &id);

private:

  Expression::Pointer GetExpression(const QString& id, const SmartPointer<IConfigurationElement>& element);

};

} // namespace berry

#endif /*BERRYDEFINITIONREGISTRY_H_*/

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

#ifndef BERRYDEFINITIONREGISTRY_H_
#define BERRYDEFINITIONREGISTRY_H_

#include "../berryExpression.h"

#include "service/berryIConfigurationElement.h"

#include <map>

namespace berry {

/**
 * This manages the extension point that allows core expression reuse.
 */
class DefinitionRegistry { //implements IRegistryChangeListener {

private:

  std::map<std::string, Expression::Pointer> cache;

  std::map<std::string, Expression::Pointer>& GetCache();

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
  Expression::Pointer GetExpression(const std::string& id);

private:

  Expression::Pointer GetExpression(const std::string& id, SmartPointer<IConfigurationElement> element);

};

} // namespace berry

#endif /*BERRYDEFINITIONREGISTRY_H_*/

/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYIITERABLE_H_
#define BERRYIITERABLE_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <org_blueberry_core_expressions_Export.h>

#include <vector>

namespace berry {

/**
 * Objects that are adaptable to <code>IIterable</code> can be used
 * as the default variable in an iterate expression.
 *
 * @see IAdaptable
 * @see IAdapterManager
 *
 * @since 3.3
 */
struct BERRY_EXPRESSIONS IIterable : public Object {

  berryObjectMacro(IIterable)

  typedef std::vector<Object::Pointer>::iterator iterator;

  virtual ~IIterable() {}

  /**
   * Returns an iterator to iterate over the elements.
   *
   * @return an iterator
   */
  virtual iterator begin() = 0;
  virtual iterator end() = 0;

};

}  // namespace berry

#endif /*BERRYIITERABLE_H_*/

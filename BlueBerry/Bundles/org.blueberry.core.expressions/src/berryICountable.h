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

#ifndef BERRYICOUNTABLE_H_
#define BERRYICOUNTABLE_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <org_blueberry_core_expressions_Export.h>

namespace berry {

/**
 * Objects that are adaptable to <code>ICountable</code> can be used
 * as the default variable in a count expression.
 *
 * @see IAdaptable
 * @see IAdapterManager
 *
 * @since 3.3
 */
struct BERRY_EXPRESSIONS ICountable : public Object {

  berryObjectMacro(ICountable)

  /**
   * Returns the number of elements.
   *
   * @return the number of elements
   */
  virtual int Count() = 0;

  virtual ~ICountable() {}

};

}

#endif /*BERRYICOUNTABLE_H_*/

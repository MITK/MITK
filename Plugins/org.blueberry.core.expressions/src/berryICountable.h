/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
 */
struct BERRY_EXPRESSIONS ICountable : public Object
{

  berryObjectMacro(ICountable);

  /**
   * Returns the number of elements.
   *
   * @return the number of elements
   */
  virtual int Count() const = 0;

};

}

Q_DECLARE_INTERFACE(berry::ICountable, "org.blueberry.core.ICountable")

#endif /*BERRYICOUNTABLE_H_*/

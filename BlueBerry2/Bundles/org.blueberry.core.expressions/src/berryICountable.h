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

#ifndef BERRYICOUNTABLE_H_
#define BERRYICOUNTABLE_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include "berryExpressionsDll.h"

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

  osgiObjectMacro(ICountable)

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

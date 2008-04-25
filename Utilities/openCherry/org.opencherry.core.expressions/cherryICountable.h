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

#ifndef CHERRYICOUNTABLE_H_
#define CHERRYICOUNTABLE_H_

#include <org.opencherry.core.runtime/cherryExpressionVariables.h>
#include <org.opencherry.osgi/cherryMacros.h>

#include "cherryExpressionsDll.h"

namespace cherry {

/**
 * Objects that are adaptable to <code>ICountable</code> can be used
 * as the default variable in a count expression.
 * 
 * @see IAdaptable
 * @see IAdapterManager
 * 
 * @since 3.3
 */
struct CHERRY_EXPRESSIONS ICountable : public ExpressionVariable {
  
  cherryClassMacro(ICountable)

  ICountable() {
    m_TypeInfo.Add("cherry::ICountable", typeid(this));
  }
  /**
   * Returns the number of elements.
   * 
   * @return the number of elements 
   */
  virtual int Count() = 0;
  
  virtual ~ICountable() {}
 
};

}

#endif /*CHERRYICOUNTABLE_H_*/

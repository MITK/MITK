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

#ifndef CHERRYIITERABLE_H_
#define CHERRYIITERABLE_H_

#include "org.opencherry.core.runtime/cherryExpressionVariables.h"

#include <vector>

namespace cherry {

/**
 * Objects that are adaptable to <code>IIterable</code> can be used
 * as the default variable in an iterate expression.
 * 
 * @see IAdaptable
 * @see IAdapterManager
 * 
 * @since 3.3
 */
struct CHERRY_API IIterable : public ExpressionVariable {

  cherryClassMacro(IIterable)
  
  typedef std::vector<ExpressionVariable::Pointer>::iterator iterator;
  
  IIterable() {
    m_TypeInfo.Add("cherry::IIterable", typeid(this));
  }
  
  virtual ~IIterable() {}
  
  /**
   * Returns an iterator to iterate over the elements.
   * 
   * @return an iterator
   */
  virtual iterator begin() = 0;
  virtual iterator end() = 0;
  
};

}  // namespace cherry

#endif /*CHERRYIITERABLE_H_*/

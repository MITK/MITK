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

#ifndef CHERRYNAMEDHANDLEOBJECT_H_
#define CHERRYNAMEDHANDLEOBJECT_H_

#include "cherryHandleObject.h"

namespace cherry {

/**
 * A handle object that carries with it a name and a description. This type of
 * handle object is quite common across the commands code base. For example,
 * <code>Command</code>, <code>Context</code> and <code>Scheme</code>.
 * 
 * @since 3.1
 */
class CHERRY_API NamedHandleObject : public HandleObject {

public:
  
  cherryClassMacro(NamedHandleObject)
  
protected:
  
  /**
   * The description for this handle. This value may be <code>null</code> if
   * the handle is undefined or has no description.
   */
  std::string description;

  /**
   * The name of this handle. This valud should not be <code>null</code>
   * unless the handle is undefined.
   */
  std::string name;

  /**
   * Constructs a new instance of <code>NamedHandleObject</code>.
   * 
   * @param id
   *            The identifier for this handle; must not be <code>null</code>.
   */
  NamedHandleObject(const std::string& id);

  
public:
  
  /**
   * Returns the description for this handle.
   * 
   * @return The description; may be <code>null</code> if there is no
   *         description.
   * @throws NotDefinedException
   *             If the handle is not currently defined.
   */
  virtual std::string GetDescription();

  /**
   * Returns the name for this handle.
   * 
   * @return The name for this handle; never <code>null</code>.
   * @throws NotDefinedException
   *             If the handle is not currently defined.
   */
  virtual std::string GetName();
};

}

#endif /*CHERRYNAMEDHANDLEOBJECT_H_*/

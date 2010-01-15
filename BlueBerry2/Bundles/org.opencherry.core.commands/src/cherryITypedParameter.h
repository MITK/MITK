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


#ifndef CHERRYITYPEDPARAMETER_H_
#define CHERRYITYPEDPARAMETER_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include "cherryCommandsDll.h"

namespace cherry {

class ParameterType;

/**
 * A command parameter that has a declared type. This interface is intended to
 * be implemented by implementors of {@link IParameter} that will support
 * parameter types.
 *
 */
struct CHERRY_COMMANDS ITypedParameter : public virtual Object {

  osgiInterfaceMacro(cherry::ITypedParameter)

  /**
   * Returns the {@link ParameterType} associated with a command parameter or
   * <code>null</code> if the parameter does not declare a type.
   * <p>
   * Note that the parameter type returned may be undefined.
   * </p>
   *
   * @return the parameter type associated with a command parameter or
   *         <code>null</code> if the parameter does not declare a type
   */
  virtual SmartPointer<ParameterType> GetParameterType() const = 0;
};

}

#endif /* CHERRYITYPEDPARAMETER_H_ */

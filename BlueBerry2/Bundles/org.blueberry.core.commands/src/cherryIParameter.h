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

#ifndef CHERRYIPARAMETER_H_
#define CHERRYIPARAMETER_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>
#include "cherryCommandsDll.h"

#include "cherryIParameterValues.h"
#include "common/cherryCommandExceptions.h"

#include <map>

namespace cherry {

using namespace osgi::framework;

/**
 * <p>
 * A parameter for a command. A parameter identifies a type of information that
 * the command might accept. For example, a "Show View" command might accept the
 * id of a view for display. This parameter also identifies possible values, for
 * display in the user interface.
 * </p>
 *
 * @since 3.1
 */
struct CHERRY_COMMANDS IParameter : public virtual Object {

  osgiInterfaceMacro(cherry::IParameter);

  /**
   * Returns the identifier for this parameter.
   *
   * @return The identifier; never <code>null</code>.
   */
  virtual std::string GetId() const = 0;

  /**
   * Returns the human-readable name for this parameter.
   *
   * @return The parameter name; never <code>null</code>.
   */
  virtual std::string GetName() const = 0;

  /**
   * Returns the values associated with this parameter.
   *
   * @return The values associated with this parameter. This must not be
   *         <code>null</code>.
   * @throws ParameterValuesException
   *             If the values can't be retrieved for some reason.
   */
  virtual IParameterValues::Pointer GetValues() const throw(ParameterValuesException) = 0;

  /**
   * Returns whether parameter is optional. Otherwise, it is required.
   *
   * @return <code>true</code> if the parameter is optional;
   *         <code>false</code> if it is required.
   */
  virtual bool IsOptional() const = 0;
};

}

#endif /*CHERRYIPARAMETER_H_*/

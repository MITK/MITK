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


#ifndef CHERRYIPARAMETERVALUES_H_
#define CHERRYIPARAMETERVALUES_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include <map>
#include <string>

namespace cherry {

using namespace osgi::framework;

/**
 * <p>
 * The parameters for a command. This interface will only be consulted if the
 * parameters need to be displayed to the user. Otherwise, they will be ignored.
 * </p>
 *
 * @since 3.1
 */
struct IParameterValues : public virtual Object {

  osgiInterfaceMacro(cherry::IParameterValues)

  typedef std::map<std::string, std::string> ParameterValuesMap;

  /**
   * Returns a map keyed by externalized names for parameter values. These
   * names should be human-readable, and are generally for display to the user
   * in a user interface of some sort. The values should be actual values that
   * will be interpreted by the handler for the command.
   *
   * @return A map of the externalizable name of the parameter value (<code>String</code>)
   *         to the actual value of the parameter (<code>String</code>).
   */
  virtual ParameterValuesMap GetParameterValues() = 0;
};

}

#endif /* CHERRYIPARAMETERVALUES_H_ */

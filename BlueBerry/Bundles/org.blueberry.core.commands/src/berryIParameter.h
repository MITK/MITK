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

#ifndef BERRYIPARAMETER_H_
#define BERRYIPARAMETER_H_

#include <berryObject.h>
#include <berryMacros.h>
#include <org_blueberry_core_commands_Export.h>


namespace berry {

struct IParameterValues;

/**
 * <p>
 * A parameter for a command. A parameter identifies a type of information that
 * the command might accept. For example, a "Show View" command might accept the
 * id of a view for display. This parameter also identifies possible values, for
 * display in the user interface.
 * </p>
 */
struct BERRY_COMMANDS IParameter : public virtual Object
{

  berryObjectMacro(berry::IParameter)

  typedef QHash<QString, QString> ParameterValues;

  /**
   * Returns the identifier for this parameter.
   *
   * @return The identifier; never <code>null</code>.
   */
  virtual QString GetId() const = 0;

  /**
   * Returns the human-readable name for this parameter.
   *
   * @return The parameter name; never <code>null</code>.
   */
  virtual QString GetName() const = 0;

  /**
   * Returns the values associated with this parameter.
   *
   * @return The values associated with this parameter. This must not be
   *         <code>null</code>.
   * @throws ParameterValuesException
   *             If the values can't be retrieved for some reason.
   */
  virtual IParameterValues* GetValues() const = 0;

  /**
   * Returns whether parameter is optional. Otherwise, it is required.
   *
   * @return <code>true</code> if the parameter is optional;
   *         <code>false</code> if it is required.
   */
  virtual bool IsOptional() const = 0;
};

}

#endif /*BERRYIPARAMETER_H_*/

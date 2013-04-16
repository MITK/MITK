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


#ifndef BERRYITYPEDPARAMETER_H_
#define BERRYITYPEDPARAMETER_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <org_blueberry_core_commands_Export.h>

namespace berry {

class ParameterType;

/**
 * A command parameter that has a declared type. This interface is intended to
 * be implemented by implementors of {@link IParameter} that will support
 * parameter types.
 *
 */
struct BERRY_COMMANDS ITypedParameter : public virtual Object {

  berryInterfaceMacro(ITypedParameter, berry)

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
  virtual SmartPointer<ParameterType> GetParameterType() = 0;
};

}

#endif /* BERRYITYPEDPARAMETER_H_ */

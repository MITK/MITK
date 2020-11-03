/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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

  berryObjectMacro(berry::ITypedParameter);

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

#endif /* BERRYITYPEDPARAMETER_H_ */

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIPARAMETERVALUES_H
#define BERRYIPARAMETERVALUES_H

#include "berryObject.h"

#include <org_blueberry_core_commands_Export.h>

#include <QHash>
#include <QString>

namespace berry {

/**
 * <p>
 * The parameters for a command. This interface will only be consulted if the
 * parameters need to be displayed to the user. Otherwise, they will be ignored.
 * </p>
*/
struct BERRY_COMMANDS IParameterValues
{
  virtual ~IParameterValues();

  /**
   * Returns a map keyed by externalized names for parameter values. These
   * names should be human-readable, and are generally for display to the user
   * in a user interface of some sort. The values should be actual values that
   * will be interpreted by the handler for the command.
   *
   * @return A map of the externalizable name of the parameter value (<code>String</code>)
   *         to the actual value of the parameter (<code>String</code>).
   */
  virtual QHash<QString,QString> GetParameterValues() const = 0;
};

}

Q_DECLARE_INTERFACE(berry::IParameterValues, "org.blueberry.core.commands.IParameterValues")

#endif // BERRYIPARAMETERVALUES_H

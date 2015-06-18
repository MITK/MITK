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

#ifndef BERRYNAMEDHANDLEOBJECT_H_
#define BERRYNAMEDHANDLEOBJECT_H_

#include "berryHandleObject.h"

namespace berry {

/**
 * A handle object that carries with it a name and a description. This type of
 * handle object is quite common across the commands code base. For example,
 * <code>Command</code>, <code>Context</code> and <code>Scheme</code>.
 *
 * @since 3.1
 */
class BERRY_COMMANDS NamedHandleObject : public HandleObject {

public:

  berryObjectMacro(NamedHandleObject);

protected:

  /**
   * The description for this handle. This value may be <code>null</code> if
   * the handle is undefined or has no description.
   */
  QString description;

  /**
   * The name of this handle. This valud should not be <code>null</code>
   * unless the handle is undefined.
   */
  QString name;

  /**
   * Constructs a new instance of <code>NamedHandleObject</code>.
   *
   * @param id
   *            The identifier for this handle; must not be <code>null</code>.
   */
  NamedHandleObject(const QString& id);


public:

  /**
   * Returns the description for this handle.
   *
   * @return The description; may be <code>null</code> if there is no
   *         description.
   * @throws NotDefinedException
   *             If the handle is not currently defined.
   */
  virtual QString GetDescription() const;

  /**
   * Returns the name for this handle.
   *
   * @return The name for this handle; never <code>null</code>.
   * @throws NotDefinedException
   *             If the handle is not currently defined.
   */
  virtual QString GetName() const;
};

}

#endif /*BERRYNAMEDHANDLEOBJECT_H_*/

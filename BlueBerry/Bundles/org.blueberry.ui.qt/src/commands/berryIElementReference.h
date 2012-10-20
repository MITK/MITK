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


#ifndef BERRYIELEMENTREFERENCE_H_
#define BERRYIELEMENTREFERENCE_H_

#include <berryObject.h>

#include <QHash>

namespace berry {

class UIElement;

/**
 * the ICommandService will return a reference for all callbacks that are
 * registered. This reference can be used to unregister the specific callback.
 * <p>
 * Similar in functionality to an IHandlerActivation.  This interface should
 * not be implemented or extended by clients.
 * </p>
 */
struct IElementReference : public Object
{

  berryObjectMacro(berry::IElementReference)

  /**
   * The command id that this callback was registered against.
   *
   * @return The command id. Will not be <code>null</code>.
   */
  virtual QString GetCommandId() const = 0;

  /**
   * The callback that was registered.
   *
   * @return Adapts to provide appropriate user feedback. Will not be
   *         <code>null</code>.
   */
  virtual SmartPointer<UIElement> GetElement() const = 0;

  /**
   * Parameters that help scope this callback registration. For example, it
   * can include parameters from the ParameterizedCommand.
   *
   * @return scoping parameters. Will not be <code>null</code>.
   */
  virtual QHash<QString, Object::Pointer> GetParameters() const = 0;
};

}

#endif /* BERRYIELEMENTREFERENCE_H_ */

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


#ifndef BERRYIELEMENTUPDATER_H
#define BERRYIELEMENTUPDATER_H

#include <berryObject.h>

namespace berry {

class UIElement;

/**
 * An IHandler for a command that expects to provide feedback through the
 * registered element mechanism must implement this interface.
 */
struct IElementUpdater : public virtual Object
{

  berryObjectMacro(berry::IElementUpdater)

  /**
   * Whenever the elements for a command are refreshed, this method is called
   * on the active handler for that command.
   * <p>
   * <b>Note:</b> Handlers must never cache the element, which can disappear
   * or be replaced at any time. Everybody should go through the
   * ICommandService refreshElements(*) method.
   * </p>
   *
   * @param element
   *            An element for a specific UI element. Will not be
   *            <code>null</code>.
   * @param parameters
   *            Any parameters registered with the callback. It may be empty.
   */
  virtual void UpdateElement(UIElement* element, const QHash<QString,Object::Pointer>& parameters) = 0;
};

}

#endif // BERRYIELEMENTUPDATER_H

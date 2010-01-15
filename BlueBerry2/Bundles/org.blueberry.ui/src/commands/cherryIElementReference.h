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


#ifndef CHERRYIELEMENTREFERENCE_H_
#define CHERRYIELEMENTREFERENCE_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include "../cherryUiDll.h"

#include <map>

namespace cherry {

class UIElement;

/**
 * the ICommandService will return a reference for all callbacks that are
 * registered. This reference can be used to unregister the specific callback.
 * <p>
 * Similar in functionality to an IHandlerActivation.  This interface should
 * not be implemented or extended by clients.
 * </p>
 *
 * @since 3.3
 */
struct CHERRY_UI IElementReference : public Object {

  osgiInterfaceMacro(cherry::IElementReference)

  /**
   * The command id that this callback was registered against.
   *
   * @return The command id. Will not be <code>null</code>.
   */
  virtual std::string GetCommandId() const = 0;

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
  virtual std::map<std::string, Object::Pointer> GetParameters() const = 0;
};

}

#endif /* CHERRYIELEMENTREFERENCE_H_ */

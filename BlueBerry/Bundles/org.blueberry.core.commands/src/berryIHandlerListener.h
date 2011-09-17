/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYIHANDLERLISTENER_H_
#define BERRYIHANDLERLISTENER_H_

#include <berryObject.h>
#include <berryMacros.h>
#include <berryMessage.h>

#include <org_blueberry_core_commands_Export.h>

namespace berry {

class HandlerEvent;

/**
 * An instance of this interface can be used by clients to receive notification
 * of changes to one or more instances of <code>IHandler</code>.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @since 3.1
 * @see IHandler#addHandlerListener(IHandlerListener)
 * @see IHandler#removeHandlerListener(IHandlerListener)
 */
struct BERRY_COMMANDS IHandlerListener : public virtual Object {

  berryInterfaceMacro(IHandlerListener, berry)

  struct Events {

    typedef Message1<SmartPointer<HandlerEvent> > Event;

    Event handlerChanged;

    void AddListener(IHandlerListener::Pointer listener);
    void RemoveListener(IHandlerListener::Pointer listener);

  private:
    typedef MessageDelegate1<IHandlerListener, SmartPointer<HandlerEvent> > Delegate;
  };

  /**
   * Notifies that one or more properties of an instance of
   * <code>IHandler</code> have changed. Specific details are described in
   * the <code>HandlerEvent</code>.
   *
   * @param handlerEvent
   *            the handler event. Guaranteed not to be <code>null</code>.
   */
  virtual void HandlerChanged(SmartPointer<HandlerEvent> handlerEvent) = 0;
};

}

#endif /* BERRYIHANDLERLISTENER_H_ */

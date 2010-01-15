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


#ifndef CHERRYEVENTDISPATCHER_H_
#define CHERRYEVENTDISPATCHER_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

namespace cherry {
namespace osgi {
namespace internal {

/**
 * The EventDispatcher interface contains the method that is called by the
 * Event Manager to complete the event delivery to the event listener.
 * <p>
 * Clients may implement this interface.
 * </p>
 * @since 3.1
 */
struct EventDispatcher : public virtual ::osgi::framework::Object {

  osgiObjectMacro(cherry::osgi::internal::EventDispatcher)

  /**
   * This method is called once for each listener.
   * This method must cast the event listener object to the appropriate listener
   * class for the event type and call the appropriate listener method.
   *
   * <p>The method should properly log/handle any exceptions thrown by the called
   * listener. The EventManager will ignore any Throwable thrown by this method
   * in order to continue delivery of the event to the next listener.
   *
   * @param eventListener This is the key in the Map.Entry for the listener.
   * The implementation of this method must cast it to the appropriate listener
   * class for the event to be delivered and the appropriate listener method
   * must then be called.
   * @param listenerObject This is the value in the Map.Entry for the listener.
   * @param eventAction This value was passed to the ListenerQueue object via one of its
   * dispatchEvent* method calls. It can provide information (such
   * as which listener method to call) so that the EventDispatcher
   * can complete the delivery of the event to the listener.
   * @param eventObject This object was passed to the ListenerQueue object via one of its
   * dispatchEvent* method calls. This object was created by the event source and
   * is passed to this method. It should contain all the necessary information (such
   * as what event object to pass) so that this method
   * can complete the delivery of the event to the listener.
   * This is typically the actual event object.
   */
  virtual void DispatchEvent(Object::Pointer eventListener, Object::Pointer listenerObject, int eventAction, Object::Pointer eventObject) = 0;
};

}
}
}

#endif /* CHERRYEVENTDISPATCHER_H_ */

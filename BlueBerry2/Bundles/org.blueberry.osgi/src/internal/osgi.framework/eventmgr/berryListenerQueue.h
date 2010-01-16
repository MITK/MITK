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


#ifndef BERRYLISTENERQUEUE_H_
#define BERRYLISTENERQUEUE_H_

#include <osgi/framework/Object.h>

#include "berryCopyOnWriteMap.h"
#include "berryEventDispatcher.h"

namespace berry {
namespace osgi {
namespace internal {

class EventManager;

using namespace ::osgi::framework;

/**
 * The ListenerQueue is used to snapshot the list of listeners at the time the event
 * is fired. The snapshot list is then used to dispatch
 * events to those listeners. A ListenerQueue object is associated with a
 * specific EventManager object. ListenerQueue objects constructed with the same
 * EventManager object will get in-order delivery of events when
 * using asynchronous delivery. No delivery order is guaranteed for synchronous
 * delivery to avoid any potential deadly embraces.
 *
 * <p>ListenerQueue objects are created as necessary to build a list of listeners
 * that should receive a specific event or events. Once the list is created, the event
 * can then be synchronously or asynchronously delivered to the list of
 * listeners. After the event has been dispatched for delivery, the
 * ListenerQueue object should be discarded as it is likely the list of listeners is stale.
 * A new ListenerQueue object should be created when it is time to deliver
 * another event. The Sets used to build the list of listeners must not change after being
 * added to the list.
 * @since 3.1
 */
class ListenerQueue {

protected:

  /**
   * EventManager with which this queue is associated.
   */
  SmartPointer<EventManager> manager;

private:

  typedef CopyOnWriteMap<std::set<Pair<Object::Pointer,Object::Pointer> > , SmartPointer<EventDispatcher> > ListenerList;
  /**
   * A list of listener lists.
   */
  ListenerList queue;

  /**
   * Once the listener queue has been used to dispatch an event,
   * you cannot add modify the queue.
   * Access to this field must be protected by a synchronized region.
   */
  mutable bool readOnly;

  mutable Poco::Mutex mutex;

public:

  ListenerQueue(const ListenerQueue& q);

  /**
   * ListenerQueue constructor. This method creates an empty snapshot list.
   *
   * @param manager The EventManager this queue is associated with.
   * @throws IllegalArgumentException If manager is null.
   */
  ListenerQueue(SmartPointer<EventManager> manager) throw(IllegalArgumentException);

  /**
   * Add a set of listeners to the snapshot list. This method can be called multiple times, prior to
   * calling one of the dispatchEvent methods, to build the list of listeners for the
   * delivery of a specific event. The specified listeners
   * are added to the snapshot list.
   *
   * @param listeners A Set of Map.Entries to add to the queue. This is typically the entrySet
   * from a CopyOnWriteIdentityMap object. This set must not change after being added to this
   * snapshot list.
   * @param dispatcher An EventDispatcher object to use when dispatching an event
   * to the specified listeners.
   * @throws IllegalStateException If called after one of the dispatch methods has been called.
   * @since 3.5
   */
  void QueueListeners(const std::set<Pair<Object::Pointer, Object::Pointer> >& listeners,
      EventDispatcher::Pointer dispatcher) throw(IllegalStateException);

  /**
   * Asynchronously dispatch an event to the snapshot list. An event dispatch thread
   * maintained by the associated EventManager is used to deliver the events.
   * This method may return immediately to the caller.
   *
   * @param eventAction This value is passed to the EventDispatcher.
   * @param eventObject This object is passed to the EventDispatcher.
   */
  void DispatchEventAsynchronous(int eventAction, Object::Pointer eventObject) const;

  /**
   * Synchronously dispatch an event to the snapshot list. The event may
   * be dispatched on the current thread or an event dispatch thread
   * maintained by the associated EventManager.
   * This method will not return to the caller until the EventDispatcher
   * has been called (and has returned) for each listener on the queue.
   *
   * @param eventAction This value is passed to the EventDispatcher.
   * @param eventObject This object is passed to the EventDispatcher.
   */
  void DispatchEventSynchronous(int eventAction, Object::Pointer eventObject) const;
};

}
}
}

#endif /* BERRYLISTENERQUEUE_H_ */

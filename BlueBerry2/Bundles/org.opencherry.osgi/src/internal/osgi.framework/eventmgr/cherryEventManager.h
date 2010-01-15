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


#ifndef CHERRYEVENTMANAGER_H_
#define CHERRYEVENTMANAGER_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include <Poco/Thread.h>
#include <Poco/SynchronizedObject.h>

#include "cherryEventDispatcher.h"
#include "cherryPair.h"

#include <set>


namespace cherry {
namespace osgi {
namespace internal {

using namespace ::osgi::framework;

/**
 * This class is the central class for the Event Manager. Each
 * program that wishes to use the Event Manager should construct
 * an EventManager object and use that object to construct
 * ListenerQueue for dispatching events. CopyOnWriteIdentityMap objects
 * must be used to manage listener lists.
 *
 * <p>This example uses the fictitious SomeEvent class and shows how to use this package
 * to deliver a SomeEvent to a set of SomeEventListeners.
 * <pre>
 *
 *  // Create an EventManager with a name for an asynchronous event dispatch thread
 *  EventManager eventManager = new EventManager("SomeEvent Async Event Dispatcher Thread");
 *  // Create a CopyOnWriteIdentityMap to hold the list of SomeEventListeners
 *  Map eventListeners = new CopyOnWriteIdentityMap();
 *
 *  // Add a SomeEventListener to the listener list
 *  eventListeners.put(someEventListener, null);
 *
 *  // Asynchronously deliver a SomeEvent to registered SomeEventListeners
 *  // Create the listener queue for this event delivery
 *  ListenerQueue listenerQueue = new ListenerQueue(eventManager);
 *  // Add the listeners to the queue and associate them with the event dispatcher
 *  listenerQueue.queueListeners(eventListeners.entrySet(), new EventDispatcher() {
 *    public void dispatchEvent(Object eventListener, Object listenerObject,
 *                                    int eventAction, Object eventObject) {
 *      try {
 *        (SomeEventListener)eventListener.someEventOccured((SomeEvent)eventObject);
 *      } catch (Throwable t) {
 *        // properly log/handle any Throwable thrown by the listener
 *      }
 *    }
 *  });
 *  // Deliver the event to the listeners.
 *  listenerQueue.dispatchEventAsynchronous(0, new SomeEvent());
 *
 *  // Remove the listener from the listener list
 *  eventListeners.remove(someEventListener);
 *
 *  // Close EventManager to clean when done to terminate async event dispatch thread.
 *  // Note that closing the event manager while asynchronously delivering events
 *  // may cause some events to not be delivered before the async event dispatch
 *  // thread terminates
 *  eventManager.close();
 * </pre>
 *
 * <p>At first glance, this package may seem more complicated than necessary
 * but it has support for some important features. The listener list supports
 * companion objects for each listener object. This is used by the OSGi framework
 * to create wrapper objects for a listener which are passed to the event dispatcher.
 * The ListenerQueue class is used to build a snap shot of the listeners prior to beginning
 * event dispatch.
 *
 * The OSGi framework uses a 2 level listener list for each listener type (4 types).
 * Level one is managed per framework instance and contains the list of BundleContexts which have
 * registered a listener. Level 2 is managed per BundleContext for the listeners in that
 * context. This allows all the listeners of a bundle to be easily and atomically removed from
 * the level one list. To use a "flat" list for all bundles would require the list to know which
 * bundle registered a listener object so that the list could be traversed when stopping a bundle
 * to remove all the bundle's listeners.
 *
 * When an event is fired, a snapshot list (ListenerQueue) must be made of the current listeners before delivery
 * is attempted. The snapshot list is necessary to allow the listener list to be modified while the
 * event is being delivered to the snapshot list. The memory cost of the snapshot list is
 * low since the ListenerQueue object uses the copy-on-write semantics
 * of the CopyOnWriteIdentityMap. This guarantees the snapshot list is never modified once created.
 *
 * The OSGi framework also uses a 2 level dispatch technique (EventDispatcher).
 * Level one dispatch is used by the framework to add the level 2 listener list of each
 * BundleContext to the snapshot in preparation for delivery of the event.
 * Level 2 dispatch is used as the final event deliverer and must cast the listener
 * and event objects to the proper type before calling the listener. Level 2 dispatch
 * will cancel delivery of an event
 * to a bundle that has stopped between the time the snapshot was created and the
 * attempt was made to deliver the event.
 *
 * <p> The highly dynamic nature of the OSGi framework had necessitated these features for
 * proper and efficient event delivery.
 * @since 3.1
 * @noextend This class is not intended to be subclassed by clients.
 */

class EventManager : public ::osgi::framework::Object {

public:

  /**
   * This class is used for asynchronously dispatching events.
   */
  class EventThread : public Poco::Thread {

  private:

    static int nextThreadNumber;
    static Poco::FastMutex threadNumberMutex;

    Poco::SynchronizedObject syncEvent;


    /**
     * Queued is a nested top-level (non-member) class. This class
     * represents the items which are placed on the asynch dispatch queue.
     * This class is private.
     */
    struct Queued {

      /** listener list for this event */
      std::set<Pair<Object::Pointer, Object::Pointer> > listeners;
      /** dispatcher of this event */
      const EventDispatcher::Pointer dispatcher;
      /** action for this event */
      const int action;
      /** object for this event */
      const Object::Pointer object;
      /** next item in event queue */
      Queued* next;

      /**
       * Constructor for event queue item
       *
       * @param l Listener list for this event
       * @param d Dispatcher for this event
       * @param a Action for this event
       * @param o Object for this event
       */
      Queued(const std::set<Pair<Object::Pointer,Object::Pointer> > l,
          EventDispatcher::Pointer d, int a, Object::Pointer o);

    };

    /** item at the head of the event queue */
    Queued* head;
    /** item at the tail of the event queue */
    Queued* tail;
    /** if false the thread must terminate */
    volatile bool running;

    static std::string GetNextName();

    /**
     * This method pulls events from
     * the queue and dispatches them.
     */
    void Run();

    /**
     * This method is called by the thread to remove
     * items from the queue so that they can be dispatched to their listeners.
     * If the queue is empty, the thread waits.
     *
     * @return The Queued removed from the top of the queue or null
     * if the thread has been requested to stop.
     */
    Queued* GetNextEvent();

  public:

    mutable Poco::Mutex mutex;

    /**
     * Constructor for the event thread.
     * @param threadName Name of the EventThread
     */
    EventThread(const std::string& threadName = "");

    /**
     * Stop thread.
     */
    void Close();

    void Start();

    /**
     * This methods takes the input parameters and creates a Queued
     * object and queues it.
     * The thread is notified.
     *
     * @param l Listener list for this event
     * @param d Dispatcher for this event
     * @param a Action for this event
     * @param o Object for this event
     */
    void PostEvent(const std::set<Pair<Object::Pointer, Object::Pointer> > l,
        EventDispatcher::Pointer d, int a, Object::Pointer o);

  };

private:

  static const bool DEBUG;

  /**
   * EventThread for asynchronous dispatch of events.
   * Access to this field must be protected by a synchronized region.
   */
  EventThread thread;

  /**
   * Once closed, an attempt to create a new EventThread will result in an
   * IllegalStateException.
   */
  bool closed;

  mutable Poco::Mutex mutex;

protected:

  /**
   * Thread name used for asynchronous event delivery
   */
  std::string threadName;

  /**
   * The thread group used for asynchronous event delivery
   */
  //ThreadGroup threadGroup;

public:

  osgiObjectMacro(cherry::osgi::internal::EventManager)

  /**
   * EventManager constructor. An EventManager object is responsible for
   * the delivery of events to listeners via an EventDispatcher.
   *
   * @param threadName The name to give the event thread associated with
   * this EventManager.  A <code>null</code> value is allowed.
   * @param threadGroup The thread group to use for the asynchronous event
   * thread associated with this EventManager. A <code>null</code> value is allowed.
   * @since 3.4
   */
  EventManager(const std::string& threadName = "");

  /**
   * This method can be called to release any resources associated with this
   * EventManager.
   * <p>
   * Closing this EventManager while it is asynchronously delivering events
   * may cause some events to not be delivered before the async event dispatch
   * thread terminates.
   */
  void Close();

  /**
   * Returns the EventThread to use for dispatching events asynchronously for
   * this EventManager.
   *
   * @return EventThread to use for dispatching events asynchronously for
   * this EventManager.
   */
  EventThread& GetEventThread();

  /**
   * This method calls the EventDispatcher object to complete the dispatch of
   * the event. If there are more elements in the list, call dispatchEvent
   * on the next item on the list.
   * This method is package private.
   *
   * @param listeners A Set of entries from a CopyOnWriteIdentityMap map.
   * @param dispatcher Call back object which is called to complete the delivery of
   * the event.
   * @param eventAction This value was passed by the event source and
   * is passed to this method. This is passed on to the call back object.
   * @param eventObject This object was created by the event source and
   * is passed to this method. This is passed on to the call back object.
   */
  static void DispatchEvent(const std::set<Pair<Object::Pointer, Object::Pointer> >& listeners,
      EventDispatcher::Pointer dispatcher, int eventAction, Object::Pointer eventObject);

};

}
}
}

#endif /* CHERRYEVENTMANAGER_H_ */

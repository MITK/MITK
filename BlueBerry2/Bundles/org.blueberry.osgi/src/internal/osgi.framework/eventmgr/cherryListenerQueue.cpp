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

#include "cherryListenerQueue.h"
#include "cherryEventManager.h"

namespace cherry {
namespace osgi {
namespace internal {

  ListenerQueue::ListenerQueue(const ListenerQueue& q)
  : manager(q.manager),
  queue(q.queue),
  readOnly(q.readOnly)
  {

  }

 ListenerQueue::ListenerQueue(EventManager::Pointer manager) throw(IllegalArgumentException) :
  manager(manager), readOnly(false)
  {
    if (!manager) {
      throw IllegalArgumentException();
    }
  }

  void ListenerQueue::QueueListeners(const std::set<Pair<Object::Pointer, Object::Pointer> >& listeners,
      EventDispatcher::Pointer dispatcher) throw(IllegalStateException) {
    Poco::Mutex::ScopedLock lock(mutex);
    if (readOnly) {
      throw IllegalStateException();
    }

    if (!listeners.empty()) {
      queue.insert(std::make_pair(listeners, dispatcher)); // enqueue the list and its dispatcher
    }
  }

  void ListenerQueue::DispatchEventAsynchronous(int eventAction, Object::Pointer eventObject) const {
    {
      Poco::Mutex::ScopedLock lock(mutex);
      readOnly = true;
    }
    EventManager::EventThread& eventThread = manager->GetEventThread();
    {
    Poco::Mutex::ScopedLock lock(eventThread.mutex); /* synchronize on the EventThread to ensure no interleaving of posting to the event thread */
      for (ListenerList::const_iterator iter = queue.begin(); iter != iter.end(); ++iter) { /* iterate over the list of listener lists */
        eventThread.PostEvent(iter->first, iter->second, eventAction, eventObject);
      }
    }
  }

  void ListenerQueue::DispatchEventSynchronous(int eventAction, Object::Pointer eventObject) const {
    {
      Poco::Mutex::ScopedLock lock(mutex);
      readOnly = true;
    }
    // We can't guarantee any delivery order for synchronous events.
    // Attempts to do so result in deadly embraces.
    for (ListenerList::const_iterator iter = queue.begin(); iter != iter.end(); ++iter) { /* iterate over the list of listener lists */
      EventManager::DispatchEvent(iter->first, iter->second, eventAction, eventObject);
    }
  }

}
}
}

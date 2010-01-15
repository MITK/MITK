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

#include "cherryEventManager.h"

#include <Poco/RunnableAdapter.h>

#include <supplement/framework.debug/cherryDebug.h>

namespace cherry
{
namespace osgi
{
namespace internal
{

const bool EventManager::DEBUG = false;

EventManager::EventManager(const std::string& threadName) :
  thread(threadName), closed(false), threadName(threadName)
{

}

void EventManager::Close()
{
  Poco::Mutex::ScopedLock lock(mutex);
  if (closed)
  {
    return;
  }
  thread.Close();
  closed = true;
}

EventManager::EventThread& EventManager::GetEventThread()
{
  Poco::Mutex::ScopedLock lock(mutex);
  if (closed)
  {
    throw IllegalStateException();
  }

  if (!thread.isRunning())
    thread.Start(); /* start the new thread */

  return thread;
}

void EventManager::DispatchEvent(const std::set<Pair<Object::Pointer,
    Object::Pointer> >& listeners, EventDispatcher::Pointer dispatcher,
    int eventAction, Object::Pointer eventObject)
{
  for (std::set<Pair<Object::Pointer, Object::Pointer> >::iterator iter =
      listeners.begin(); iter != listeners.end(); ++iter)
  { /* iterate over the list of listeners */
    try
    {
      /* Call the EventDispatcher to complete the delivery of the event. */
      dispatcher->DispatchEvent(iter->first, iter->second, eventAction,
          eventObject);
    } catch (const std::exception* e)
    {
      /* Consume and ignore any exceptions thrown by the listener */
      if (DEBUG)
      {
        std::cout << "Exception in " << iter->first << std::endl;
        Debug::PrintStackTrace();
      }
    }
  }
}

int EventManager::EventThread::nextThreadNumber = 0;
Poco::FastMutex EventManager::EventThread::threadNumberMutex;

EventManager::EventThread::Queued::Queued(const std::set<Pair<Object::Pointer,
    Object::Pointer> > l, EventDispatcher::Pointer d, int a, Object::Pointer o) :
  listeners(l), dispatcher(d), action(a), object(0), next(0)
{

}

std::string EventManager::EventThread::GetNextName()
{
  Poco::FastMutex::ScopedLock lock(threadNumberMutex);
  return "EventManagerThread-" + nextThreadNumber++;
}

void EventManager::EventThread::Run()
{
  Queued* item = 0;
  try
  {
    while (true)
    {
      item = GetNextEvent();
      if (item == 0)
      {
        return;
      }
      EventManager::DispatchEvent(item->listeners, item->dispatcher,
          item->action, item->object);
      delete item;
    }
  } catch (...)
  {
    delete item;
    if (EventManager::DEBUG)
    {
      Debug::PrintStackTrace();
    }
    throw ;
  }
}

EventManager::EventThread::Queued* EventManager::EventThread::GetNextEvent()
{
  Poco::Mutex::ScopedLock lock(mutex);
  while (running && (head == 0))
  {
    syncEvent.wait();
  }

  if (!running)
  { /* if we are stopping */
    return 0;
  }

  Queued* item = head;
  head = item->next;
  if (head == 0)
  {
    tail = 0;
  }

  return item;
}

EventManager::EventThread::EventThread(const std::string& threadName)
: Poco::Thread(threadName.empty() ? GetNextName() : threadName),
head(0), tail(0), running(true)
{
  // SetDaemon(true); /* Mark thread as daemon thread */
}

void EventManager::EventThread::Close()
{
  running = false;
  syncEvent.notify();
}

void EventManager::EventThread::Start()
{
  Poco::RunnableAdapter<EventThread> runnable(*this, &EventThread::Run);
  this->start(runnable);
}

void EventManager::EventThread::PostEvent(const std::set<Pair<Object::Pointer, Object::Pointer> > l,
    EventDispatcher::Pointer d, int a, Object::Pointer o)
{
  Poco::Mutex::ScopedLock lock(mutex);

  if (!isRunning())
  { /* If the thread is not alive, throw an exception */
    throw IllegalStateException();
  }

  Queued* item = new Queued(l, d, a, o);

  if (head == 0) /* if the queue was empty */
  {
    head = item;
    tail = item;
  }
  else /* else add to end of queue */
  {
    tail->next = item;
    tail = item;
  }

  syncEvent.notify();
}

}
}
}

//
// EventDispatcher.cpp
//
// $Id$
//
// Library: XML
// Package: DOM
// Module:  DOMEvents
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/DOM/EventDispatcher.h"
#include "Poco/DOM/Event.h"
#include "Poco/DOM/EventListener.h"


namespace 
{
	class DispatchGuard
	{
	public:
		DispatchGuard(int& count):
			_count(count)
		{
			++_count;
		}
		
		~DispatchGuard()
		{
			--_count;
		}
		
	private:
		int& _count;
	};
}


namespace Poco {
namespace XML {


EventDispatcher::EventDispatcher():
	_inDispatch(0)
{
}

	
EventDispatcher::~EventDispatcher()
{
}


void EventDispatcher::addEventListener(const XMLString& type, EventListener* listener, bool useCapture)
{
	EventListenerItem item;
	item.type       = type;
	item.pListener  = listener;
	item.useCapture = useCapture;
	_listeners.push_front(item);
}


void EventDispatcher::removeEventListener(const XMLString& type, EventListener* listener, bool useCapture)
{
	EventListenerList::iterator it = _listeners.begin();
	while (it != _listeners.end())
	{
		if (it->type == type && it->pListener == listener && it->useCapture == useCapture)
		{
			it->pListener = 0;
		}
		if (!_inDispatch && !it->pListener)
		{
			EventListenerList::iterator del = it++;
			_listeners.erase(del);
		}
		else ++it;
	}
}


void EventDispatcher::dispatchEvent(Event* evt)
{
	DispatchGuard guard(_inDispatch);
	EventListenerList::iterator it = _listeners.begin();
	while (it != _listeners.end())
	{
		if (it->pListener && it->type == evt->type())
		{
			it->pListener->handleEvent(evt);
		}
		if (!it->pListener)
		{
			EventListenerList::iterator del = it++;
			_listeners.erase(del);
		}
		else ++it;
	}
}


void EventDispatcher::captureEvent(Event* evt)
{
	DispatchGuard guard(_inDispatch);
	EventListenerList::iterator it = _listeners.begin();
	while (it != _listeners.end())
	{
		if (it->pListener && it->useCapture && it->type == evt->type())
		{
			it->pListener->handleEvent(evt);
		}
		if (!it->pListener)
		{
			EventListenerList::iterator del = it++;
			_listeners.erase(del);
		}
		else ++it;
	}
}


void EventDispatcher::bubbleEvent(Event* evt)
{
	DispatchGuard guard(_inDispatch);
	EventListenerList::iterator it = _listeners.begin();
	while (it != _listeners.end())
	{
		if (it->pListener && !it->useCapture && it->type == evt->type())
		{
			it->pListener->handleEvent(evt);
		}
		if (!it->pListener)
		{
			EventListenerList::iterator del = it++;
			_listeners.erase(del);
		}
		else ++it;
	}
}


} } // namespace Poco::XML

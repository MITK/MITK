/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#ifndef QXTNATIVEEVENTFILTER_H
#define QXTNATIVEEVENTFILTER_H

#include "qxtapplication.h"

/*!
    \class QxtNativeEventFilter QxtNativeEventFilter
    \ingroup QxtGui
    \brief A native event filter to access platform specific events.

    QxtNativeEventFilter provides access to platform specific native events
    without the need of subclassing QApplication.

    \note QxtNativeEventFilter requires QxtApplication.

    Example usage:
    \code
    qxtApp->installNativeEventFilter(myObject);

    class MyObject : public QxtNativeEventFilter {
        public:
            ...

            bool x11EventFilter(XEvent* event) {
                if (event->type == ...) {
                    ...
                }
                return false;
            }

            bool winEventFilter(MSG* msg, long* result) {
                if (msg->message == ...) {
                    ...
                }
                return false;
            }

            bool macEventFilter(EventHandlerCallRef caller, EventRef event) {
                if (GetEventClass(event) == ...) {
                    ...
                }
                return false;
            }
    };
    \endcode

    \sa QxtApplication::installNativeEventFilter()
 */

/*!
    \fn QxtNativeEventFilter::x11EventFilter(XEvent* event)

    Filters X11 events if this object has been installed as a native event filter.
    In your reimplementation of this function, if you want to filter the event out,
    i.e. stop it being handled further, return \b true; otherwise return \b false.

    \sa QxtApplication::installNativeEventFilter()
 */

/*!
    \fn QxtNativeEventFilter::winEventFilter(MSG* msg, long* result)

    Filters Windows events if this object has been installed as a native event filter.
    In your reimplementation of this function, if you want to filter the event out,
    i.e. stop it being handled further, return \b true; otherwise return \b false.

    \sa QxtApplication::installNativeEventFilter()
 */

/*!
    \fn QxtNativeEventFilter::macEventFilter(EventHandlerCallRef caller, EventRef event)

    Filters Mac events if this object has been installed as a native event filter.
    In your reimplementation of this function, if you want to filter the event out,
    i.e. stop it being handled further, return \b true; otherwise return \b false.

    \sa QxtApplication::installNativeEventFilter()
 */

class QxtNativeEventFilter
{
public:
    virtual ~QxtNativeEventFilter()
    {
        qxtApp->removeNativeEventFilter(this);
    }

#if defined(Q_WS_X11)
    virtual bool x11EventFilter(XEvent*)
    {
        return false;
    }
#elif defined(Q_WS_WIN)
    virtual bool winEventFilter(MSG*, long*)
    {
        return false;
    }
#elif defined(Q_WS_MAC)
    virtual bool macEventFilter(EventHandlerCallRef, EventRef)
    {
        return false;
    }
#endif // Q_WS_*
};

#endif // QXTNATIVEEVENTFILTER_H

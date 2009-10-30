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
#ifndef QXTGUI_H_INCLUDED
#define QXTGUI_H_INCLUDED

#if !defined(Q_WS_QWS)
#include "qxtapplication.h"
#endif
#include "qxtbasespinbox.h"
#include "qxtcheckcombobox.h"
#include "qxtconfigdialog.h"
#include "qxtconfirmationmessage.h"
#include "qxtcountrycombobox.h"
#include "qxtcountrymodel.h"
#include "qxtdockwidget.h"
#include "qxtflowview.h"
#include "qxtglobalshortcut.h"
#include "qxtgroupbox.h"
#include "qxtheaderview.h"
#include "qxtitemdelegate.h"
#include "qxtitemeditorcreator.h"
#include "qxtitemeditorcreatorbase.h"
#include "qxtlabel.h"
#include "qxtlanguagecombobox.h"
#include "qxtletterboxwidget.h"
#include "qxtlistwidget.h"
#include "qxtlistwidgetitem.h"
#include "qxtnativeeventfilter.h"
#include "qxtprogresslabel.h"
#include "qxtproxystyle.h"
#include "qxtpushbutton.h"
#include "qxtscheduleheaderwidget.h"
#include "qxtscheduleitemdelegate.h"
#include "qxtscheduleview.h"
#include "qxtspanslider.h"
#include "qxtstandarditemeditorcreator.h"
#include "qxtstars.h"
#include "qxtstringspinbox.h"
#include "qxtstringvalidator.h"
#include "qxtstyleoptionscheduleviewitem.h"
#include "qxttabbar.h"
#include "qxttablewidget.h"
#include "qxttablewidgetitem.h"
#include "qxttabwidget.h"
#include "qxttooltip.h"
#include "qxttreewidget.h"
#include "qxttreewidgetitem.h"
#if !defined(Q_WS_QWS) && !defined(Q_WS_MAC)
#include "qxtwindowsystem.h"
#endif
#include "qxtsortfilterproxymodel.h"

/** \defgroup QxtGui QxtGui
    \brief    The QxtGui module extends QtGui.
*/
#endif // QXTGUI_H_INCLUDED

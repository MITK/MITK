/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
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

#ifndef QXTGLOBAL_H
#define QXTGLOBAL_H

#include <QtGlobal>
#ifdef Q_OS_WIN
#include <qxtconfig.h>
#endif

#define QXT_VERSION 0x000501
#define QXT_VERSION_STR "0.5.1"

//--------------------------global macros------------------------------

#ifndef QXT_NO_MACROS

#endif // QXT_NO_MACROS

//--------------------------export macros------------------------------

#define QXT_DLLEXPORT DO_NOT_USE_THIS_ANYMORE

#if defined(QXT_SHARED)
#    if defined(BUILD_QXT_CORE)
#        define QXT_CORE_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_CORE_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_CORE_EXPORT
#endif // BUILD_QXT_CORE

#if defined(QXT_SHARED)
#    if defined(BUILD_QXT_GUI)
#        define QXT_GUI_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_GUI_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_GUI_EXPORT
#endif // BUILD_QXT_GUI

#if defined(QXT_SHARED)
#    if defined(BUILD_QXT_NETWORK)
#        define QXT_NETWORK_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_NETWORK_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_NETWORK_EXPORT
#endif // BUILD_QXT_NETWORK

#if defined(QXT_SHARED)
#    if defined(BUILD_QXT_SQL)
#        define QXT_SQL_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_SQL_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_SQL_EXPORT
#endif // BUILD_QXT_SQL

#if defined(QXT_SHARED)
#    if defined(BUILD_QXT_WEB)
#        define QXT_WEB_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_WEB_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_WEB_EXPORT
#endif // BUILD_QXT_WEB

#if defined(QXT_SHARED)
#    if defined(BUILD_QXT_CRYPTO)
#        define QXT_CRYPTO_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_CRYPTO_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_CRYPTO_EXPORT
#endif // BUILD_QXT_CRYPTO

#if defined(QXT_SHARED)
#    if defined(BUILD_QXT_BERKELEY)
#        define QXT_BERKELEY_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_BERKELEY_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_BERKELEY_EXPORT
#endif // BUILD_QXT_BERKELEY

#if defined BUILD_QXT_CORE || defined BUILD_QXT_GUI || defined BUILD_QXT_MEDIA || defined  BUILD_QXT_SQL || defined BUILD_QXT_NETWORK || defined BUILD_QXT_KIT || defined BUILD_QXT_WEB || defined BUILD_QXT_CRYPTO || defined BUILD_QXT_BERKELEY
#   define BUILD_QXT
#endif

QXT_CORE_EXPORT const char* qxtVersion();

#ifndef QT_BEGIN_NAMESPACE
#define QT_BEGIN_NAMESPACE
#endif

#ifndef QT_END_NAMESPACE
#define QT_END_NAMESPACE
#endif

#ifndef QT_FORWARD_DECLARE_CLASS
#define QT_FORWARD_DECLARE_CLASS(Class) class Class;
#endif

#endif // QXT_GLOBAL

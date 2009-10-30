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

#ifndef QXTNAMESPACE_H
#define QXTNAMESPACE_H

#include <qxtglobal.h>


/**
    \namespace Qxt
    \ingroup QxtCore
    \brief The Qxt Namespace

    The Qxt Namespace

*/


#if defined BUILD_QXT | defined Q_MOC_RUN
#include <QObject>

class QXT_CORE_EXPORT Qxt  : public QObject
{
    Q_OBJECT
    Q_ENUMS(Rotation)
    Q_ENUMS(DecorationStyle)
    Q_ENUMS(ErrorCode)


public:
#else
namespace Qxt
{
#endif




    /*!
    \enum Qxt::Rotation
    This enum describes the rotation.
    */
    enum Rotation
    {
        NoRotation  = 0,  /*!< No rotation. */
        UpsideDown  = 180,  /*!< Upside down (180 degrees). */
        Clockwise  = 90,  /*!< Clockwise (90 degrees). */
        CounterClockwise = 270  /*!< CounterClockwise (-90 degrees).  */
    };

    /*!
    \enum Qxt::DecorationStyle

    This enum describes the decoration style.
    */

    enum DecorationStyle
    {
        NoDecoration,    /*!< No decoration */
        Buttonlike,    /*!< A style like in Qt Designer's Widget Box. */
        Menulike    /*!< A menu alike style. */
    };


    /*!
    \enum Qxt::ErrorCode
    The error Code dropped by QxtError
    */
    enum ErrorCode
    {
        NoError,    /*!< everything fine  */
        UnknownError,    /*!< a not defined error */
        LogicalError,    /*!< something happened that makes no sense (e.g. you tryed to compare apples and bananas) */
        Bug,     /*!< that should not happen. please report any ocurence of it. */
        UnexpectedEndOfFunction,  /*!< suddenly the function returned where it should not. please report this behaiviour */
        NotImplemented,             /*!< the feature you requested has not been implemented for this situation. this could also mean you are trying to compare apples and bananas. */
        CodecError,    /*!< Something went wrong with some codec. Please check if your codec is supported */
        NotInitialised,    /*!< Somone tryed to call a function of an object that needs to be start() ed  or something like it. */
        EndOfFile,    /*!< The end of the Input has been reached. There is no more data. */
        FileIOError,    /*!< File Input/Output Error*/
        FormatError,    /*!< The Format of the input is corupted or not supported. */   //10
        DeviceError,    /*!< The Device of the Computer reported failure or we are not able to comunicate with it.*/
        SDLError,    /*!< SDL reported an error */
        InsufficientMemory,   /*!< Not enough memory to perform the action. */
        SeeErrorString,    /*!< The Error has no definition here, but the Error String should tell you more. This is mostly used for third party errors. */
        UnexpectedNullParameter,  /*!< NULL was passed where it shouldn't. */
        ClientTimeout,    /*!< The Client didn't answer within the expected time rang. */
        SocketIOError,    /*!< Socket Input/Output Error*/
        ParserError,    /*!< Unable to parse the requested string or file.*/
        HeaderTooLong,    /*!< The Header that was passed for parsing was too long.*/
        Auth,     /*!< Authentication failed.*/
        Overflow    /*!< Overflow protection triggered.*/

    };

    /*!
    \enum Qxt::QxtItemDataRole
    Data roles for qxt models/views
    */
    enum QxtItemDataRole
    {
        ItemStartTimeRole  = Qt::UserRole + 1,        /*!< The starttime of a item in timebased views */
        ItemDurationRole   = ItemStartTimeRole + 1,   /*!< The duration of a item in timebased views  */
        UserRole           = ItemDurationRole + 23    /*!< The first role that can be used for application-specific purposes.  */
    };

    /*!
    \enum Qxt::Timeunit
    */
    enum Timeunit
    {
        Second,
        Minute,
        Hour,
        Day,
        Week,
        Month,
        Year
    };

};

#endif // QXTNAMESPACE_H

/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef MITKSERVICEINTERFACE_H
#define MITKSERVICEINTERFACE_H

template <class T> inline const char* mitk_service_interface_iid()
{ return 0; }

#if defined(QT_DEBUG) || defined(QT_NODEBUG)
#include <qobject.h>

#define MITK_DECLARE_SERVICE_INTERFACE(IFace, IId)                               \
  template <> inline const char* qobject_interface_iid<IFace *>()                \
  { return IId; }                                                                \
  template <> inline const char* mitk_service_interface_iid<IFace *>()           \
  { return IId; }                                                                \
  template <> inline IFace *qobject_cast<IFace *>(QObject *object)               \
  { return reinterpret_cast<IFace *>((object ? object->qt_metacast(IId) : 0)); } \
  template <> inline IFace *qobject_cast<IFace *>(const QObject *object)         \
  { return reinterpret_cast<IFace *>((object ? const_cast<QObject *>(object)->qt_metacast(IId) : 0)); }

#else

#define MITK_DECLARE_SERVICE_INTERFACE(IFace, IId)                               \
  template <> inline const char* mitk_service_interface_iid<IFace *>()           \
  { return IId; }                                                                \

#endif

#endif // MITKSERVICEINTERFACE_H

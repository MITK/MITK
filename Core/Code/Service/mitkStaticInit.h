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

Extracted from qglobal.h from Qt 4.7.3 and adapted for MITK.
Original copyright (c) Nokia Corporation. Usage covered by the
GNU Lesser General Public License version 2.1
(http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html) and the Nokia Qt
LGPL Exception version 1.1 (file LGPL_EXCEPTION.txt in Qt 4.7.3 package).

=========================================================================*/

#ifndef MITK_STATIC_INIT_H
#define MITK_STATIC_INIT_H

#include <itkSimpleFastMutexLock.h>

namespace mitk {

// POD for MITK_GLOBAL_STATIC
template <typename T>
class GlobalStatic
{
public:
  T* pointer;
  bool destroyed;

  // Guards acces to "pointer". If a "atomic pointer" class is available,
  // this should be used instead of an explicit mutex.
  itk::SimpleFastMutexLock mutex;
};

template<typename T>
struct DefaultGlobalStaticDeleter
{
  void operator()(GlobalStatic<T>& globalStatic) const
  {
    delete globalStatic.pointer;
    globalStatic.pointer = 0;
    globalStatic.destroyed = true;
  }
};

// Created as a function-local static to delete a GlobalStatic<T>
template <typename T, template<typename T_> class Deleter = DefaultGlobalStaticDeleter>
class GlobalStaticDeleter
{
public:
  GlobalStatic<T> &globalStatic;

  GlobalStaticDeleter(GlobalStatic<T> &_globalStatic)
    : globalStatic(_globalStatic)
  { }

  inline ~GlobalStaticDeleter()
  {
    Deleter<T> deleter;
    deleter(globalStatic);
  }
};

} // namespace mitk

#define MITK_GLOBAL_STATIC_INIT(TYPE, NAME)                              \
  static ::mitk::GlobalStatic<TYPE>& this_##NAME()                       \
  {                                                                      \
    static ::mitk::GlobalStatic<TYPE> l =                                \
        { 0, false, itk::SimpleFastMutexLock() };                        \
    return l;                                                            \
  }

#define MITK_GLOBAL_STATIC(TYPE, NAME)                                   \
  MITK_GLOBAL_STATIC_INIT(TYPE, NAME)                                    \
  static TYPE *NAME()                                                    \
  {                                                                      \
    if (!this_##NAME().pointer && !this_##NAME().destroyed)              \
    {                                                                    \
      TYPE *x = new TYPE;                                                \
      bool ok = false;                                                   \
      {                                                                  \
        this_##NAME().mutex.Lock();                                      \
        if (!this_##NAME().pointer)                                      \
        {                                                                \
          this_##NAME().pointer = x;                                     \
          ok = true;                                                     \
        }                                                                \
        this_##NAME().mutex.Unlock();                                    \
      }                                                                  \
      if (!ok)                                                           \
        delete x;                                                        \
      else                                                               \
        static ::mitk::GlobalStaticDeleter<TYPE > cleanup(this_##NAME());\
    }                                                                    \
    return this_##NAME().pointer;                                        \
  }

#define MITK_GLOBAL_STATIC_WITH_DELETER(TYPE, NAME, DELETER)             \
  MITK_GLOBAL_STATIC_INIT(TYPE, NAME)                                    \
  static TYPE *NAME()                                                    \
  {                                                                      \
    if (!this_##NAME().pointer && !this_##NAME().destroyed)              \
    {                                                                    \
      TYPE *x = new TYPE;                                                \
      bool ok = false;                                                   \
      {                                                                  \
        this_##NAME().mutex.Lock();                                      \
        if (!this_##NAME().pointer)                                      \
        {                                                                \
          this_##NAME().pointer = x;                                     \
          ok = true;                                                     \
        }                                                                \
        this_##NAME().mutex.Unlock();                                    \
      }                                                                  \
      if (!ok)                                                           \
        delete x;                                                        \
      else                                                               \
        static ::mitk::GlobalStaticDeleter<TYPE, DELETER > cleanup(this_##NAME());\
    }                                                                    \
    return this_##NAME().pointer;                                        \
  }

#define MITK_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                   \
  MITK_GLOBAL_STATIC_INIT(TYPE, NAME)                                    \
  static TYPE *NAME()                                                    \
  {                                                                      \
    if (!this_##NAME().pointer && !this_##NAME().destroyed)              \
    {                                                                    \
      TYPE *x = new TYPE ARGS;                                           \
      bool ok = false;                                                   \
      {                                                                  \
        this_##NAME().mutex.Lock();                                      \
        if (!this_##NAME().pointer)                                      \
        {                                                                \
          this_##NAME().pointer = x;                                     \
          ok = true;                                                     \
        }                                                                \
        this_##NAME().mutex.Unlock();                                    \
      }                                                                  \
      if (!ok)                                                           \
        delete x;                                                        \
      else                                                               \
        static ::mitk::GlobalStaticDeleter<TYPE > cleanup(this_##NAME());\
    }                                                                    \
    return this_##NAME().pointer;                                        \
  }

#endif // MITK_STATIC_INIT_H

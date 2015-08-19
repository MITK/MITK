/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmlMitkBigRenderLock.h"

#include "QVTKQuickItem.h"

QMutex& QmlMitkBigRenderLock::GetMutex()
{
  static QMutex globalRenderingLock;
  return globalRenderingLock;
}

QmlMitkBigRenderLock::QmlMitkBigRenderLock(QObject* parent)
:QObject(parent)
{
}

bool QmlMitkBigRenderLock::eventFilter(QObject* /*obj*/, QEvent* /*event*/)
{
  /*
    TODO

    this should BLOCK during rendering, i.e.

    it should acquire a mutex lock which is also acquired by the rendering method in QVTKQuickItem

    TODO:

    THIS MUST BE CHANGED!

    It is NOT sufficient to delay signal delivery, BUT we must block rendering until the signal is completely processed.
    TODO: what would happen if a signal receiver calls rendering->ForceImmediateUpdateAll ?



  */

  if ( GetMutex().tryLock() )
  {
    // ok, let's continue
  }
  else
  {
    GetMutex().lock();
  }

  GetMutex().unlock();

  return false; // don't actually filter/remove events
}

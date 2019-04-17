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

#ifndef QMITKSAFENOTIFY_H
#define QMITKSAFENOTIFY_H

#include <mitkException.h>
#include <mitkLogMacros.h>

#include <QMessageBox>

template<class A>
bool QmitkSafeNotify(A* app, QObject* receiver, QEvent* event)
{
  return app->A::notify(receiver, event);
}

#endif // QMITKSAFENOTIFY_H

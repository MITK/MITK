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

#ifndef __QmlMitkBigRenderLock_h
#define __QmlMitkBigRenderLock_h

#include <QtCore>

/**
  \brief Workaround lock around MITK rendering.

  QtQuick renders in a thread, MITK datastructures do not
  tolerate this well. The current work-around is a big
  lock that delays signal delivery while rendering is in
  progress.

  The proper solution would be to make data structures
  in rendering thread safe. This solution is much more
  work though, so it will come later.
*/
class  QmlMitkBigRenderLock : public QObject
{
  Q_OBJECT
  public:

    static QMutex& GetMutex();

    QmlMitkBigRenderLock(QObject* parent = 0);

  protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif

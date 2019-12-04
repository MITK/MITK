/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSingleApplication.h"
#include "QmitkSafeNotify.h"

QmitkSingleApplication::QmitkSingleApplication(int &argc, char **argv, bool safeMode)
  : QtSingleApplication(argc, argv), m_SafeMode(safeMode)
{
}

bool QmitkSingleApplication::notify(QObject *receiver, QEvent *event)
{
  return m_SafeMode
    ? QmitkSafeNotify<QtSingleApplication>(this, receiver, event)
    : QtSingleApplication::notify(receiver, event);
}

void QmitkSingleApplication::setSafeMode(bool safeMode)
{
  m_SafeMode = safeMode;
}

bool QmitkSingleApplication::getSafeMode() const
{
  return m_SafeMode;
}

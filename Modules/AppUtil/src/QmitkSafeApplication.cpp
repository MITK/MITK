/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSafeApplication.h"

#include "QmitkSafeNotify.h"

#include <mitkException.h>
#include <mitkLogMacros.h>

QmitkSafeApplication::QmitkSafeApplication(int &argc, char **argv, bool safeMode)
  : QApplication(argc, argv),
    m_SafeMode(safeMode)
{
}

bool QmitkSafeApplication::notify(QObject *receiver, QEvent *event)
{
  return m_SafeMode
    ? QmitkSafeNotify<QApplication>(this, receiver, event)
    : QApplication::notify(receiver, event);
}

void QmitkSafeApplication::setSafeMode(bool safeMode)
{
  m_SafeMode = safeMode;
}

bool QmitkSafeApplication::getSafeMode() const
{
  return m_SafeMode;
}

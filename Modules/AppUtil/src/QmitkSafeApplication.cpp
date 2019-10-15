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

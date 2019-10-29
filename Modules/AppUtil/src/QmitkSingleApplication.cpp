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

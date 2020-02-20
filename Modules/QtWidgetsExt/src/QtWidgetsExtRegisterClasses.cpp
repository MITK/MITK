/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QtWidgetsExtRegisterClasses.h"
#include "QmitkCallbackFromGUIThread.h"

void QtWidgetsExtRegisterClasses()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "QmitkExtRegisterClasses()";

    static QmitkCallbackFromGUIThread globalQmitkCallbackFromGUIThread;

    alreadyDone = true;
  }
}

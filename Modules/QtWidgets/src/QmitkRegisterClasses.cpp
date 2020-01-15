/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRegisterClasses.h"

#include "QmitkApplicationCursor.h"
#include "QmitkRenderingManagerFactory.h"

#include <iostream>

void QmitkRegisterClasses()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "QmitkRegisterClasses()";

    // We have to put this in a file containing a class that is directly used
    // somewhere. Otherwise, e.g. when put in VtkRenderWindowInteractor.cpp,
    // it is removed by the linker.

    // Create and register RenderingManagerFactory for this platform.
    static QmitkRenderingManagerFactory qmitkRenderingManagerFactory;
    static QmitkApplicationCursor globalQmitkApplicationCursor; // create one instance

    alreadyDone = true;
  }
}

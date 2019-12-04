/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryUIException.h"

namespace berry {

CTK_IMPLEMENT_EXCEPTION(UIException, PlatformException, "UI exception")

CTK_IMPLEMENT_EXCEPTION(WorkbenchException, UIException, "Workbench error")

CTK_IMPLEMENT_EXCEPTION(PartInitException, WorkbenchException, "Part initialization error")

}

/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryUIException.h"

namespace berry {

CTK_IMPLEMENT_EXCEPTION(UIException, PlatformException, "UI exception")

CTK_IMPLEMENT_EXCEPTION(WorkbenchException, UIException, "Workbench error")

CTK_IMPLEMENT_EXCEPTION(PartInitException, WorkbenchException, "Part initialization error")

}

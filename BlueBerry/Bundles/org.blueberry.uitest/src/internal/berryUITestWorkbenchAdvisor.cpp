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

#include "berryUITestWorkbenchAdvisor.h"
#include "berryUITestWorkbenchWindowAdvisor.h"

namespace berry
{

UITestWorkbenchAdvisor::UITestWorkbenchAdvisor()
{

}

WorkbenchWindowAdvisor* UITestWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
    IWorkbenchWindowConfigurer::Pointer configurer)
{
  return new UITestWorkbenchWindowAdvisor(configurer);
}

std::string UITestWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return "";
}

}

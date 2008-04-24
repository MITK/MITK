/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "cherryPlatformUI.h"

#include "org.opencherry.osgi/cherryPlatform.h"
#include "org.opencherry.osgi/service/cherryIConfigurationElement.h"
#include "org.opencherry.osgi/service/cherryIExtensionPointService.h"

#include "cherryUIException.h"

#include <vector>

namespace cherry {

const std::string PlatformUI::PLUGIN_ID = "org.opencherry.ui";
const std::string PlatformUI::XP_WORKBENCH = PlatformUI::PLUGIN_ID + ".workbench";
const std::string PlatformUI::XP_VIEWS = PlatformUI::PLUGIN_ID + ".views";

Workbench::Pointer PlatformUI::m_Workbench(0);

int
PlatformUI::CreateAndRunWorkbench()
{
  std::vector<IConfigurationElement::Pointer> extensions(
    Platform::GetExtensionPointService()->GetConfigurationElementsFor(PlatformUI::XP_WORKBENCH));
  
  for (unsigned int i = 0; i < extensions.size(); ++i)
  {
    if (extensions[i]->GetName() == "workbench")
    {
      m_Workbench = extensions[i]->CreateExecutableExtension<cherry::Workbench>("cherryWorkbench", "class");
      m_Workbench->Run();
      return 0;
    }
  }
  
  throw WorkbenchException("No registered workbench extension found");
}
  
IWorkbench::Pointer 
PlatformUI::GetWorkbench()
{
  return m_Workbench;
}
  
bool 
PlatformUI::IsWorkbenchRunning()
{
  return !m_Workbench.IsNull();
}

}

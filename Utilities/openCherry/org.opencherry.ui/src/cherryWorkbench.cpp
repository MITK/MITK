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

#include "cherryWorkbench.h"

#include "internal/cherryViewRegistry.h"
#include "internal/cherryEditorRegistry.h"

#include "internal/cherryWorkbenchPlugin.h"

namespace cherry {

const std::string Workbench::DIALOG_ID_SHOW_VIEW = "org.opencherry.ui.dialogs.showview";


Workbench::Workbench()
{
  
}

Workbench::~Workbench()
{
 
}

IViewRegistry*
Workbench::GetViewRegistry()
{
  return WorkbenchPlugin::GetDefault()->GetViewRegistry();
}

IEditorRegistry*
Workbench::GetEditorRegistry()
{
  return WorkbenchPlugin::GetDefault()->GetEditorRegistry();
}
  
void 
Workbench::ShowPerspective(const std::string& /*perspectiveId*/)
{
  
}

}

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

#include "berryPerspectiveRegistryReader.h"

#include "berryPerspectiveRegistry.h"
#include "berryWorkbenchPlugin.h"
#include "berryPlatformUI.h"
#include "berryWorkbenchRegistryConstants.h"

namespace berry
{
PerspectiveRegistryReader::PerspectiveRegistryReader()
{
}

void PerspectiveRegistryReader::ReadPerspectives(PerspectiveRegistry* out)
{
  registry = out;
  this->ReadRegistry(PlatformUI::PLUGIN_ID,
      WorkbenchRegistryConstants::PL_PERSPECTIVES);
}

bool PerspectiveRegistryReader::ReadElement(IConfigurationElement::Pointer element)
{
  if (element->GetName() == WorkbenchRegistryConstants::TAG_PERSPECTIVE)
  {
    try
    {
      std::string id;
      element->GetAttribute(WorkbenchRegistryConstants::ATT_ID, id);
      std::vector<berry::IConfigurationElement::Pointer> childs = element->GetChildren("description");
      PerspectiveDescriptor::Pointer desc(
          new PerspectiveDescriptor(id, element));
      if (childs.size() > 0)
      {
        desc->SetDescription(childs[0]->GetValue());
      }
      registry->AddPerspective(desc);
    }
    catch (CoreException e)
    {
      // log an error since its not safe to open a dialog here
      WorkbenchPlugin::Log("Unable to create layout descriptor.", e);//$NON-NLS-1$
    }
    return true;
  }

  return false;
}
}
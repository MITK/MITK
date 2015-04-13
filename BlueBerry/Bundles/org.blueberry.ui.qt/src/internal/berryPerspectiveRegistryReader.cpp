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

PerspectiveRegistryReader::PerspectiveRegistryReader(PerspectiveRegistry* out)
  : registry(out)
{
}

void PerspectiveRegistryReader::ReadPerspectives(IExtensionRegistry* in)
{
  this->ReadRegistry(in, PlatformUI::PLUGIN_ID(),
                     WorkbenchRegistryConstants::PL_PERSPECTIVES);
}

bool PerspectiveRegistryReader::ReadElement(const IConfigurationElement::Pointer &element)
{
  if (element->GetName() == WorkbenchRegistryConstants::TAG_PERSPECTIVE)
  {
    try
    {
      QString id = element->GetAttribute(WorkbenchRegistryConstants::ATT_ID);
      PerspectiveDescriptor::Pointer desc(
          new PerspectiveDescriptor(id, element));
      QList<berry::IConfigurationElement::Pointer> childs = element->GetChildren("description");
      if (!childs.isEmpty())
      {
        desc->SetDescription(childs[0]->GetValue());
      }
      registry->AddPerspective(desc);
    }
    catch (const CoreException& e)
    {
      // log an error since its not safe to open a dialog here
      WorkbenchPlugin::Log("Unable to create layout descriptor.", e);//$NON-NLS-1$
    }
    return true;
  }

  return false;
}

}

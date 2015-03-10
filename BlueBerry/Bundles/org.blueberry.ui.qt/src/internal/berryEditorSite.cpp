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

#include "berryEditorSite.h"

#include "berryPartPane.h"

#include <cassert>

namespace berry
{

QString EditorSite::GetInitialScopeId()
{
  return "org.blueberry.ui.textEditorScope"; //$NON-NLS-1$
}

EditorSite::EditorSite(IEditorReference::Pointer ref,
    IEditorPart::Pointer editor, WorkbenchPage* page,
    EditorDescriptor::Pointer d)
: PartSite(ref, editor, page), desc(d)
{
  assert(!desc.IsNull());

  if (!desc->GetConfigurationElement().IsNull())
  {
    this->SetConfigurationElement(desc->GetConfigurationElement());
  }
  else
  {
    // system external and in-place editors do not have a corresponding configuration element
    this->SetId(desc->GetId());
    this->SetRegisteredName(desc->GetLabel());
  }

  // Initialize the services specific to this editor site.
  //initializeDefaultServices();
}

IEditorPart::Pointer EditorSite::GetEditorPart()
{
  return this->GetPart().Cast<IEditorPart>();
}

EditorDescriptor::Pointer EditorSite::GetEditorDescriptor()
{
  return desc;
}

}

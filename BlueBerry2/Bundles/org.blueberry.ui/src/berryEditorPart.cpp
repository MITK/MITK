/*=========================================================================

Program:   BlueBerry Platform
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

#include "berryEditorPart.h"

#include "berryImageDescriptor.h"

#include <cassert>

namespace berry
{

EditorPart::EditorPart()
{
}

void EditorPart::SetInput(IEditorInput::Pointer input)
{
  editorInput = input;
}

void EditorPart::SetInputWithNotify(IEditorInput::Pointer input)
{
  if (input != editorInput)
  {
    editorInput = input;
    //firePropertyChange(PROP_INPUT);
  }
}

void EditorPart::SetContentDescription(const std::string& description)
{
//  if (compatibilityTitleListener != null)
//  {
//    removePropertyListener(compatibilityTitleListener);
//    compatibilityTitleListener = null;
//  }

  WorkbenchPart::SetContentDescription(description);
}

void EditorPart::SetPartName(const std::string& partName)
{
//  if (compatibilityTitleListener != null)
//  {
//    removePropertyListener(compatibilityTitleListener);
//    compatibilityTitleListener = null;
//  }

  WorkbenchPart::SetPartName(partName);
}

void EditorPart::CheckSite(IWorkbenchPartSite::Pointer site)
{
  WorkbenchPart::CheckSite(site);
  assert(!site.Cast<IEditorSite>().IsNull()); // The site for an editor must be an IEditorSite
}

IEditorInput::Pointer EditorPart::GetEditorInput() const
{
  return editorInput;
}

IEditorSite::Pointer EditorPart::GetEditorSite() const
{
  return this->GetSite().Cast<IEditorSite>();
}

std::string EditorPart::GetTitleToolTip() const
{
  if (editorInput.IsNull())
  {
    return WorkbenchPart::GetTitleToolTip();
  }
  else
  {
    return editorInput->GetToolTipText();
  }
}

bool EditorPart::IsSaveOnCloseNeeded() const
{
  return this->IsDirty();
}


}

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryEditorPart.h"

#include "berryIWorkbenchPartConstants.h"

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
    FirePropertyChange(IWorkbenchPartConstants::PROP_INPUT);
  }
}

void EditorPart::SetContentDescription(const QString& description)
{
//  if (compatibilityTitleListener != null)
//  {
//    removePropertyListener(compatibilityTitleListener);
//    compatibilityTitleListener = null;
//  }

  WorkbenchPart::SetContentDescription(description);
}

void EditorPart::SetPartName(const QString& partName)
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

QString EditorPart::GetTitleToolTip() const
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

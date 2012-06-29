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

#include "berryNullEditorInput.h"
#include "berryPartPane.h"


namespace berry
{

NullEditorInput::NullEditorInput()
{
}

NullEditorInput::NullEditorInput(EditorReference::Pointer editorReference)
{
  //poco_assert(editorReference.IsNotNull());
  this->editorReference = editorReference;
}

bool NullEditorInput::Exists() const
{
  return false;
}

std::string NullEditorInput::GetName() const
{
  if (editorReference.IsNotNull())
    return editorReference->GetName();
  return ""; //$NON-NLS-1$
}

std::string NullEditorInput::GetToolTipText() const
{
  if (editorReference.IsNotNull())
    return editorReference->GetTitleToolTip();
  return ""; //$NON-NLS-1$
}

bool NullEditorInput::operator==(const Object* o) const
{
  const NullEditorInput* input = dynamic_cast<const NullEditorInput*>(o);
  if (input == 0) return false;

  return true;
}

}

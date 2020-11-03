/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryNullEditorInput.h"

#include "berryPartPane.h"
#include "berryAbstractUICTKPlugin.h"

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

QIcon NullEditorInput::GetIcon() const
{
  return AbstractUICTKPlugin::GetMissingIcon();
}

QString NullEditorInput::GetName() const
{
  if (editorReference.IsNotNull())
    return editorReference->GetName();
  return "";
}

const IPersistableElement* NullEditorInput::GetPersistable() const
{
  return nullptr;
}

QString NullEditorInput::GetToolTipText() const
{
  if (editorReference.IsNotNull())
    return editorReference->GetTitleToolTip();
  return "";
}

Object*NullEditorInput::GetAdapter(const QString& /*adapterType*/) const
{
  return nullptr;
}

bool NullEditorInput::operator==(const Object* o) const
{
  const NullEditorInput* input = dynamic_cast<const NullEditorInput*>(o);
  if (input == nullptr) return false;

  return true;
}

}

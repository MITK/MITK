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

#include "berryFileEditorInput.h"

#include <QIcon>

namespace berry {

FileEditorInput::FileEditorInput(const QString& path)
 : m_Path(path)
{

}

QString FileEditorInput::GetPath() const
{
   return m_Path.absolutePath();
}

bool FileEditorInput::Exists() const
{
  return m_Path.exists();
}

QString FileEditorInput::GetName() const
{
  return m_Path.fileName();
}

QString FileEditorInput::GetToolTipText() const
{
  return m_Path.absolutePath();
}

QIcon FileEditorInput::GetIcon() const
{
  return QIcon();
}

const IPersistableElement*FileEditorInput::GetPersistable() const
{
  return nullptr;
}

Object* FileEditorInput::GetAdapter(const QString& /*adapterType*/) const
{
  return nullptr;
}

bool FileEditorInput::operator==(const Object* o) const
{
  if (const IPathEditorInput* other = dynamic_cast<const IPathEditorInput*>(o))
  {
    return other->GetPath() == this->GetPath();
  }
  return false;
}

}


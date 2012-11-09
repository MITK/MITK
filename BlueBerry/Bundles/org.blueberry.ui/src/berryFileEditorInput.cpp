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

#include <Poco/File.h>

namespace berry {

FileEditorInput::FileEditorInput(const Poco::Path& path)
 : m_Path(path)
{

}

Poco::Path FileEditorInput::GetPath() const
{
   return m_Path;
}

bool FileEditorInput::Exists() const
{
  Poco::File file(m_Path);
  return file.exists();
}

std::string FileEditorInput::GetName() const
{
  return m_Path.getFileName();
}

std::string FileEditorInput::GetToolTipText() const
{
  return m_Path.toString();
}

bool FileEditorInput::operator==(const Object* o) const
{
  if (const IPathEditorInput* other = dynamic_cast<const IPathEditorInput*>(o))
  {
    return other->GetPath().toString() == this->GetPath().toString();
  }

  return false;
}

}


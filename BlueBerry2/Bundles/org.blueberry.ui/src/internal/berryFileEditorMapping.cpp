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

#include "berryFileEditorMapping.h"

namespace berry
{

const std::string FileEditorMapping::STAR = "*"; //$NON-NLS-1$
const std::string FileEditorMapping::DOT = "."; //$NON-NLS-1$

bool FileEditorMapping::CompareList (
    const std::list<IEditorDescriptor::Pointer>& l1,
    const std::list<IEditorDescriptor::Pointer>& l2) const
{
  if (l1.size() != l2.size())
  {
    return false;
  }

  std::list<IEditorDescriptor::Pointer>::const_iterator iter1, iter2;
  for (iter1 = l1.begin(), iter2 = l2.begin(); iter1 != l1.end() && iter2 != l2.end();
       ++iter1, ++iter2)
  {
    if (!(iter1->IsNull() ? iter2->IsNull() : *iter1 == *iter2))
    {
      return false;
    }
  }

  return true;
}


FileEditorMapping::FileEditorMapping(const std::string& extension, const std::string& name)
{
  if (name.size() < 1)
  {
    this->SetName(STAR);
  }
  else
  {
    this->SetName(name);
  }

  this->SetExtension(extension);
}

void FileEditorMapping::AddEditor(EditorDescriptor::Pointer editor)
{
  editors.push_back(editor);
  deletedEditors.remove(editor);
}

bool FileEditorMapping::operator==(const Object* obj) const
{
  if (const FileEditorMapping* mapping = dynamic_cast<const FileEditorMapping*>(obj))
  {
    if (this == obj)
  {
    return true;
  }

  if (this->name != mapping->GetName())
  {
    return false;
  }
  if (this->extension != mapping->GetExtension())
  {
    return false;
  }

  if (!this->CompareList(this->editors, mapping->GetEditors()))
  {
    return false;
  }

  return this->CompareList(this->deletedEditors, mapping->deletedEditors);
  }

  return false;
}

IEditorDescriptor::Pointer FileEditorMapping::GetDefaultEditor()
{

  if (editors.size() == 0) // || WorkbenchActivityHelper.restrictUseOf(editors.get(0)))
  {
    return IEditorDescriptor::Pointer(0);
  }

  return editors.front();
}

std::list<IEditorDescriptor::Pointer> FileEditorMapping::GetEditors() const
{
  return editors;
}

std::list<IEditorDescriptor::Pointer> FileEditorMapping::GetDeletedEditors() const
{
  return deletedEditors;
}

std::string FileEditorMapping::GetExtension() const
{
  return extension;
}

std::string FileEditorMapping::GetLabel() const
{
  return name + (extension.empty() ? "" : DOT + extension); //$NON-NLS-1$
}

std::string FileEditorMapping::GetName() const
{
  return name;
}

void FileEditorMapping::RemoveEditor(EditorDescriptor::Pointer editor)
{
  editors.remove(editor);
  deletedEditors.push_back(editor);
  declaredDefaultEditors.remove(editor);
}

void FileEditorMapping::SetDefaultEditor(EditorDescriptor::Pointer editor)
{
  editors.remove(editor);
  editors.push_front(editor);
  declaredDefaultEditors.remove(editor);
  declaredDefaultEditors.push_front(editor);
}

void FileEditorMapping::SetEditorsList(const std::list<IEditorDescriptor::Pointer>& newEditors)
{
  editors = newEditors;
  declaredDefaultEditors = newEditors;
}

void FileEditorMapping::SetDeletedEditorsList(const std::list<IEditorDescriptor::Pointer>& newDeletedEditors)
{
  deletedEditors = newDeletedEditors;
}

void FileEditorMapping::SetExtension(const std::string& extension)
{
  this->extension = extension;
}

void FileEditorMapping::SetName(const std::string& name)
{
  this->name = name;
}

std::list<IEditorDescriptor::Pointer> FileEditorMapping::GetDeclaredDefaultEditors()
{
  return declaredDefaultEditors;
}

bool FileEditorMapping::IsDeclaredDefaultEditor(IEditorDescriptor::Pointer editor)
{
  for (std::list<IEditorDescriptor::Pointer>::iterator iter = declaredDefaultEditors.begin();
       iter != declaredDefaultEditors.end(); ++iter)
  {
    if (*iter == editor) return true;
  }

  return false;
}

void FileEditorMapping::SetDefaultEditors(const std::list<IEditorDescriptor::Pointer>& defaultEditors)
{
  declaredDefaultEditors = defaultEditors;
}

}

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryFileEditorMapping.h"

namespace berry
{

const QString FileEditorMapping::STAR = "*"; //$NON-NLS-1$
const QString FileEditorMapping::DOT = "."; //$NON-NLS-1$

bool FileEditorMapping::CompareList (
    const QList<IEditorDescriptor::Pointer>& l1,
    const QList<IEditorDescriptor::Pointer>& l2) const
{
  if (l1.size() != l2.size())
  {
    return false;
  }

  QList<IEditorDescriptor::Pointer>::const_iterator iter1, iter2;
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


FileEditorMapping::FileEditorMapping(const QString& extension, const QString& name)
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
  deletedEditors.removeAll(editor);
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
    return IEditorDescriptor::Pointer(nullptr);
  }

  return editors.front();
}

QList<IEditorDescriptor::Pointer> FileEditorMapping::GetEditors() const
{
  return editors;
}

QList<IEditorDescriptor::Pointer> FileEditorMapping::GetDeletedEditors() const
{
  return deletedEditors;
}

QString FileEditorMapping::GetExtension() const
{
  return extension;
}

QString FileEditorMapping::GetLabel() const
{
  return name + (extension.isEmpty() ? "" : DOT + extension);
}

QString FileEditorMapping::GetName() const
{
  return name;
}

void FileEditorMapping::RemoveEditor(EditorDescriptor::Pointer editor)
{
  editors.removeAll(editor);
  deletedEditors.push_back(editor);
  declaredDefaultEditors.removeAll(editor);
}

void FileEditorMapping::SetDefaultEditor(EditorDescriptor::Pointer editor)
{
  editors.removeAll(editor);
  editors.push_front(editor);
  declaredDefaultEditors.removeAll(editor);
  declaredDefaultEditors.push_front(editor);
}

void FileEditorMapping::SetEditorsList(const QList<IEditorDescriptor::Pointer>& newEditors)
{
  editors = newEditors;
  declaredDefaultEditors = newEditors;
}

void FileEditorMapping::SetDeletedEditorsList(const QList<IEditorDescriptor::Pointer>& newDeletedEditors)
{
  deletedEditors = newDeletedEditors;
}

void FileEditorMapping::SetExtension(const QString& extension)
{
  this->extension = extension;
}

void FileEditorMapping::SetName(const QString& name)
{
  this->name = name;
}

QList<IEditorDescriptor::Pointer> FileEditorMapping::GetDeclaredDefaultEditors()
{
  return declaredDefaultEditors;
}

bool FileEditorMapping::IsDeclaredDefaultEditor(IEditorDescriptor::Pointer editor)
{
  for (QList<IEditorDescriptor::Pointer>::iterator iter = declaredDefaultEditors.begin();
       iter != declaredDefaultEditors.end(); ++iter)
  {
    if (*iter == editor) return true;
  }

  return false;
}

void FileEditorMapping::SetDefaultEditors(const QList<IEditorDescriptor::Pointer>& defaultEditors)
{
  declaredDefaultEditors = defaultEditors;
}

}

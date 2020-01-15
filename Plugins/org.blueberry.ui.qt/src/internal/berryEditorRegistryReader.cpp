/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryEditorRegistryReader.h"

#include "berryEditorRegistry.h"
#include "berryEditorDescriptor.h"
#include "berryWorkbenchRegistryConstants.h"
#include "berryPlatformUI.h"
#include "berryPlatform.h"

#include <QStringList>

namespace berry
{

void EditorRegistryReader::AddEditors(EditorRegistry* registry)
{
  this->editorRegistry = registry;
  this->ReadRegistry(Platform::GetExtensionRegistry(),
                     PlatformUI::PLUGIN_ID(),
                     WorkbenchRegistryConstants::PL_EDITOR);
}

bool EditorRegistryReader::ReadElement(const IConfigurationElement::Pointer& element)
{
  if (element->GetName() != WorkbenchRegistryConstants::TAG_EDITOR)
  {
    return false;
  }

  QString id = element->GetAttribute(WorkbenchRegistryConstants::ATT_ID);
  if (id.isEmpty())
  {
    this->LogMissingAttribute(element, WorkbenchRegistryConstants::ATT_ID);
    return true;
  }

  EditorDescriptor::Pointer editor(new EditorDescriptor(id, element));

  QList<QString> extensionsVector;
  QList<QString> filenamesVector;
  QList<QString> contentTypeVector;
  bool defaultEditor = false;

  QString value = element->GetAttribute(WorkbenchRegistryConstants::ATT_NAME);

  // Get editor name (required field).
  if (value.isEmpty())
  {
    this->LogMissingAttribute(element, WorkbenchRegistryConstants::ATT_NAME);
    return true;
  }

  // Get target extensions (optional field)
  QString extensionsString = element->GetAttribute(WorkbenchRegistryConstants::ATT_EXTENSIONS);
  if (!extensionsString.isEmpty())
  {
    QStringList tokens = extensionsString.split(',', QString::SkipEmptyParts);
    foreach(QString token, tokens)
    {
      extensionsVector.push_back(token.trimmed());
    }
  }
  QString filenamesString = element->GetAttribute(WorkbenchRegistryConstants::ATT_FILENAMES);
  if (!filenamesString.isEmpty())
  {
    QStringList tokens = filenamesString.split(',', QString::SkipEmptyParts);
    foreach(QString token, tokens)
    {
      filenamesVector.push_back(token.trimmed());
    }
  }

  QList<IConfigurationElement::Pointer> bindings = element->GetChildren(
                                                     WorkbenchRegistryConstants::TAG_CONTENT_TYPE_BINDING);
  for (int i = 0; i < bindings.size(); ++i)
  {
    QString contentTypeId = bindings[i]->GetAttribute(
                              WorkbenchRegistryConstants::ATT_CONTENT_TYPE_ID);
    if (contentTypeId.isEmpty())
    {
      continue;
    }
    contentTypeVector.push_back(contentTypeId);
  }

  // Is this the default editor?
  defaultEditor = element->GetAttribute(WorkbenchRegistryConstants::ATT_DEFAULT).compare("true", Qt::CaseInsensitive) == 0;

  // Add the editor to the manager.
  editorRegistry->AddEditorFromPlugin(editor, extensionsVector,
      filenamesVector, contentTypeVector, defaultEditor);
  return true;
}

void EditorRegistryReader::ReadElement(EditorRegistry* editorRegistry,
                                       const IConfigurationElement::Pointer& element)
{
  this->editorRegistry = editorRegistry;
  this->ReadElement(element);
}

}

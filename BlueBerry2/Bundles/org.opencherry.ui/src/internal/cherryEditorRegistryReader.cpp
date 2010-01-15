/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryEditorRegistryReader.h"

#include "cherryEditorRegistry.h"
#include "cherryEditorDescriptor.h"
#include "cherryWorkbenchRegistryConstants.h"
#include "../cherryPlatformUI.h"

#include <Poco/String.h>
#include <Poco/StringTokenizer.h>

namespace cherry
{

void EditorRegistryReader::AddEditors(EditorRegistry* registry)
{
  this->editorRegistry = registry;
  this->ReadRegistry(PlatformUI::PLUGIN_ID,
      WorkbenchRegistryConstants::PL_EDITOR);
}

bool EditorRegistryReader::ReadElement(IConfigurationElement::Pointer element)
{
  if (element->GetName() != WorkbenchRegistryConstants::TAG_EDITOR)
  {
    return false;
  }

  std::string id;
  if (!element->GetAttribute(WorkbenchRegistryConstants::ATT_ID, id))
  {
    this->LogMissingAttribute(element, WorkbenchRegistryConstants::ATT_ID);
    return true;
  }

  EditorDescriptor::Pointer editor(new EditorDescriptor(id, element));

  std::vector<std::string> extensionsVector;
  std::vector<std::string> filenamesVector;
  std::vector<std::string> contentTypeVector;
  bool defaultEditor = false;

  std::string value;

  // Get editor name (required field).
  if (!element->GetAttribute(WorkbenchRegistryConstants::ATT_NAME, value))
  {
    this->LogMissingAttribute(element, WorkbenchRegistryConstants::ATT_NAME);
    return true;
  }

  // Get target extensions (optional field)
  std::string extensionsString;
  if (element->GetAttribute(WorkbenchRegistryConstants::ATT_EXTENSIONS,
      extensionsString))
  {
    Poco::StringTokenizer tokenizer(extensionsString, ",",
        Poco::StringTokenizer::TOK_IGNORE_EMPTY
            | Poco::StringTokenizer::TOK_TRIM);//$NON-NLS-1$
    Poco::StringTokenizer::Iterator token = tokenizer.begin();
    while (token != tokenizer.end())
    {
      extensionsVector.push_back(*token);
      ++token;
    }
  }
  std::string filenamesString;
  if (element->GetAttribute(WorkbenchRegistryConstants::ATT_FILENAMES,
      filenamesString))
  {
    Poco::StringTokenizer tokenizer(filenamesString, ",",
        Poco::StringTokenizer::TOK_IGNORE_EMPTY
            | Poco::StringTokenizer::TOK_TRIM);//$NON-NLS-1$
    Poco::StringTokenizer::Iterator token = tokenizer.begin();
    while (token != tokenizer.end())
    {
      filenamesVector.push_back(*token);
      ++token;
    }
  }

  IConfigurationElement::vector bindings = element->GetChildren(
      WorkbenchRegistryConstants::TAG_CONTENT_TYPE_BINDING);
  for (unsigned int i = 0; i < bindings.size(); ++i)
  {
    std::string contentTypeId;
    if (!bindings[i]->GetAttribute(
        WorkbenchRegistryConstants::ATT_CONTENT_TYPE_ID, contentTypeId))
    {
      continue;
    }
    contentTypeVector.push_back(contentTypeId);
  }

  // Is this the default editor?
  element->GetBoolAttribute(WorkbenchRegistryConstants::ATT_DEFAULT, defaultEditor);

  // Add the editor to the manager.
  editorRegistry->AddEditorFromPlugin(editor, extensionsVector,
      filenamesVector, contentTypeVector, defaultEditor);
  return true;
}

void EditorRegistryReader::ReadElement(EditorRegistry* editorRegistry,
    IConfigurationElement::Pointer element)
{
  this->editorRegistry = editorRegistry;
  this->ReadElement(element);
}

}

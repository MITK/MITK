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

#include "cherryEditorDescriptor.h"

#include "cherryWorkbenchRegistryConstants.h"
#include "cherryWorkbenchPlugin.h"

#include "../cherryEditorPart.h"

namespace cherry
{

const int EditorDescriptor::OPEN_INTERNAL = 0x01;

const int EditorDescriptor::OPEN_INPLACE = 0x02;

const int EditorDescriptor::OPEN_EXTERNAL = 0x04;

EditorDescriptor::EditorDescriptor(const std::string& id2,
    IConfigurationElement::Pointer element)
: matchingStrategyChecked(false), openMode(0)
{
  this->SetID(id2);
  this->SetConfigurationElement(element);
}

EditorDescriptor::EditorDescriptor() : matchingStrategyChecked(false), openMode(0)
{
}

//IEditorActionBarContributor::Pointer EditorDescriptor::CreateActionBarContributor()
//{
//  // Handle case for predefined editor descriptors, like the
//  // one for IEditorRegistry.SYSTEM_INPLACE_EDITOR_ID, which
//  // don't have a configuration element.
//  if (configurationElement.IsNull())
//  {
//    return IEditorActionBarContributor::Pointer();
//  }
//
//  // Get the contributor class name.
//  std::string className;
//  if (!configurationElement->GetAttribute(IWorkbenchRegistryConstants::ATT_CONTRIBUTOR_CLASS, className))
//  {
//    return IEditorActionBarContributor::Pointer();
//  }
//
//  // Create the contributor object.
//  IEditorActionBarContributor contributor;
//  try
//  {
//    contributor = configurationElement->CreateExecutableExtension<IEditorActionBarContributor>(
//        IWorkbenchRegistryConstants::ATT_CONTRIBUTOR_CLASS);
//  }
//  catch (CoreException e)
//  {
//    WorkbenchPlugin::Log("Unable to create editor contributor: " + //$NON-NLS-1$
//        id, e);
//  }
//  return contributor;
//}

std::string EditorDescriptor::GetClassName()
{
  if (configurationElement.IsNull())
  {
    return className;
  }
  return RegistryReader::GetClassValue(configurationElement,
      WorkbenchRegistryConstants::ATT_CLASS);
}

IConfigurationElement::Pointer EditorDescriptor::GetConfigurationElement()
{
  return configurationElement;
}

IEditorPart::Pointer EditorDescriptor::CreateEditor()
{
  IEditorPart::Pointer extension = configurationElement->CreateExecutableExtension<IEditorPart>(
      WorkbenchRegistryConstants::ATT_CLASS);
  return extension;
}

std::string EditorDescriptor::GetFileName()
{
  //if (program == null)
  //{
    if (configurationElement.IsNull())
    {
      return fileName;
    }
    std::string val;
    configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_COMMAND, val);
    return val;
  //}
  //return program.getName();
}

std::string EditorDescriptor::GetId() const
{
  //if (program == null)
  //{
    if (configurationElement.IsNull())
    {
      return id;
    }
    std::string val;
    configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_ID, val);
    return val;
  //}
  //return Util.safeString(program.getName());
}

std::string EditorDescriptor::GetLabel()
{
  //if (program == null)
  //{
    if (configurationElement.IsNull())
    {
      return editorName;
    }
    std::string val;
    configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME, val);
    return val;
  //}
  //return program.getName();
}

std::string EditorDescriptor::GetLauncher()
{
  if (configurationElement.IsNull())
  {
    return launcherName;
  }
  std::string val;
  configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_LAUNCHER, val);
  return val;
}

std::string EditorDescriptor::GetPluginID()
{
  if (!configurationElement.IsNull())
  {
    return configurationElement->GetContributor();
  }
  return pluginIdentifier;
}

bool EditorDescriptor::IsInternal()
{
  return this->GetOpenMode() == OPEN_INTERNAL;
}

bool EditorDescriptor::IsOpenInPlace()
{
  return this->GetOpenMode() == OPEN_INPLACE;
}

bool EditorDescriptor::IsOpenExternal()
{
  return this->GetOpenMode() == OPEN_EXTERNAL;
}

bool EditorDescriptor::LoadValues(IMemento::Pointer memento)
{
//  editorName = memento.getString(IWorkbenchConstants.TAG_LABEL);
//  imageFilename = memento.getString(IWorkbenchConstants.TAG_IMAGE);
//  className = memento.getString(IWorkbenchConstants.TAG_CLASS);
//  launcherName = memento.getString(IWorkbenchConstants.TAG_LAUNCHER);
//  fileName = memento.getString(IWorkbenchConstants.TAG_FILE);
//  id = Util.safeString(memento.getString(IWorkbenchConstants.TAG_ID));
//  pluginIdentifier = memento.getString(IWorkbenchConstants.TAG_PLUGIN);
//
//  Integer openModeInt = memento
//  .getInteger(IWorkbenchConstants.TAG_OPEN_MODE);
//  if (openModeInt != null)
//  {
//    openMode = openModeInt.intValue();
//  }
//  else
//  {
//    // legacy: handle the older attribute names, needed to allow reading of pre-3.0-RCP workspaces
//    boolean internal = new Boolean(memento
//        .getString(IWorkbenchConstants.TAG_INTERNAL))
//    .booleanValue();
//    boolean openInPlace = new Boolean(memento
//        .getString(IWorkbenchConstants.TAG_OPEN_IN_PLACE))
//    .booleanValue();
//    if (internal)
//    {
//      openMode = OPEN_INTERNAL;
//    }
//    else
//    {
//      if (openInPlace)
//      {
//        openMode = OPEN_INPLACE;
//      }
//      else
//      {
//        openMode = OPEN_EXTERNAL;
//      }
//    }
//  }
//  if (openMode != OPEN_EXTERNAL && openMode != OPEN_INTERNAL && openMode
//      != OPEN_INPLACE)
//  {
//    WorkbenchPlugin
//    .log("Ignoring editor descriptor with invalid openMode: " + this); //$NON-NLS-1$
//    return false;
//  }
//
//  String programName = memento
//  .getString(IWorkbenchConstants.TAG_PROGRAM_NAME);
//  if (programName != null)
//  {
//    this.program = findProgram(programName);
//  }
  return true;
}

void EditorDescriptor::SaveValues(IMemento::Pointer memento)
{
//  memento.putString(IWorkbenchConstants.TAG_LABEL, getLabel());
//  memento.putString(IWorkbenchConstants.TAG_IMAGE, getImageFilename());
//  memento.putString(IWorkbenchConstants.TAG_CLASS, getClassName());
//  memento.putString(IWorkbenchConstants.TAG_LAUNCHER, getLauncher());
//  memento.putString(IWorkbenchConstants.TAG_FILE, getFileName());
//  memento.putString(IWorkbenchConstants.TAG_ID, getId());
//  memento.putString(IWorkbenchConstants.TAG_PLUGIN, getPluginId());
//
//  memento.putInteger(IWorkbenchConstants.TAG_OPEN_MODE, getOpenMode());
//  // legacy: handle the older attribute names, needed to allow reading of workspace by pre-3.0-RCP eclipses
//  memento.putString(IWorkbenchConstants.TAG_INTERNAL, String
//  .valueOf(isInternal()));
//  memento.putString(IWorkbenchConstants.TAG_OPEN_IN_PLACE, String
//  .valueOf(isOpenInPlace()));
//
//  if (this.program != null)
//  {
//    memento.putString(IWorkbenchConstants.TAG_PROGRAM_NAME,
//        this.program.getName());
//  }
}

int EditorDescriptor::GetOpenMode()
{
  if (configurationElement.IsNull())
  { // if we've been serialized, return our serialized value
    return openMode;
  }
  else if (this->GetLauncher() != "")
  {
    // open using a launcer
    return EditorDescriptor::OPEN_EXTERNAL;
  }
  else if (this->GetFileName() != "")
  {
    // open using an external editor
    return EditorDescriptor::OPEN_EXTERNAL;
  }
  else if (this->GetPluginId() != "")
  {
    // open using an internal editor
    return EditorDescriptor::OPEN_INTERNAL;
  }
  else
  {
    return 0; // default for system editor
  }
}

void EditorDescriptor::SetClassName(const std::string& newClassName)
{
  className = newClassName;
}

void EditorDescriptor::SetConfigurationElement(
    IConfigurationElement::Pointer newConfigurationElement)
{
  configurationElement = newConfigurationElement;
}

void EditorDescriptor::SetFileName(const std::string& aFileName)
{
  fileName = aFileName;
}

void EditorDescriptor::SetID(const std::string& anID)
{
  id = anID;
}

void EditorDescriptor::SetLauncher(const std::string& newLauncher)
{
  launcherName = newLauncher;
}

void EditorDescriptor::SetName(const std::string& newName)
{
  editorName = newName;
}

void EditorDescriptor::SetOpenMode(int mode)
{
  openMode = mode;
}

void EditorDescriptor::SetPluginIdentifier(const std::string& anID)
{
  pluginIdentifier = anID;
}

std::string EditorDescriptor::ToString()
{
  return "EditorDescriptor(id=" + this->GetId() + ", label=" + this->GetLabel() + ")"; //$NON-NLS-2$ //$NON-NLS-3$//$NON-NLS-1$
}

std::string EditorDescriptor::GetLocalId()
{
  return this->GetId();
}

std::string EditorDescriptor::GetPluginId()
{
  return this->GetPluginID();
}

IEditorMatchingStrategy::Pointer EditorDescriptor::GetEditorMatchingStrategy()
{
  if (matchingStrategy.IsNull() && !matchingStrategyChecked)
  {
    matchingStrategyChecked = true;
    if (/*program == null &&*/ !configurationElement.IsNull())
    {
      std::string strategy;
      if (configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_MATCHING_STRATEGY, strategy))
      {
        try
        {
          matchingStrategy = configurationElement->CreateExecutableExtension<IEditorMatchingStrategy>(WorkbenchRegistryConstants::ATT_MATCHING_STRATEGY);
        }
        catch (CoreException e)
        {
          WorkbenchPlugin::Log("Error creating editor management policy for editor id " + this->GetId(), e); //$NON-NLS-1$
        }
      }
    }
  }
  return matchingStrategy;
}

}

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

#include "berryEditorDescriptor.h"

#include "berryWorkbenchRegistryConstants.h"
#include "berryWorkbenchPlugin.h"

#include "berryEditorPart.h"
#include "berryIContributor.h"

namespace berry
{

const int EditorDescriptor::OPEN_INTERNAL = 0x01;

const int EditorDescriptor::OPEN_INPLACE = 0x02;

const int EditorDescriptor::OPEN_EXTERNAL = 0x04;

EditorDescriptor::EditorDescriptor(const QString& id2,
    IConfigurationElement::Pointer element) :
  testImage(true), matchingStrategyChecked(false), openMode(0)
{
  this->SetID(id2);
  this->SetConfigurationElement(element);
}

EditorDescriptor::EditorDescriptor() :
  testImage(true), matchingStrategyChecked(false), openMode(0)
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
//  QString className;
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

QString EditorDescriptor::GetEditorClassName() const
{
  if (configurationElement.IsNull())
  {
    return className;
  }
  return RegistryReader::GetClassValue(configurationElement,
      WorkbenchRegistryConstants::ATT_CLASS);
}

IConfigurationElement::Pointer EditorDescriptor::GetConfigurationElement() const
{
  return configurationElement;
}

IEditorPart::Pointer EditorDescriptor::CreateEditor()
{
  IEditorPart::Pointer editorPart(configurationElement->CreateExecutableExtension<IEditorPart> (
          WorkbenchRegistryConstants::ATT_CLASS));
  return editorPart;
}

QString EditorDescriptor::GetFileName() const
{
  //if (program == null)
  //{
  if (configurationElement.IsNull())
  {
    return fileName;
  }
  return configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_COMMAND);
  //}
  //return program.getName();
}

QString EditorDescriptor::GetId() const
{
  //if (program == null)
  //{
  if (configurationElement.IsNull())
  {
    return id;
  }
  return configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_ID);
  //}
  //return Util.safeString(program.getName());
}

QIcon EditorDescriptor::GetImageDescriptor() const
{
  if (testImage)
  {
    testImage = false;
    if (imageDesc.isNull())
    {
      QString imageFileName(this->GetImageFilename());
      QString command(this->GetFileName());
      if (!imageFileName.isEmpty() && configurationElement)
      {
        imageDesc = AbstractUICTKPlugin::ImageDescriptorFromPlugin(
              configurationElement->GetContributor()->GetName(), imageFileName);
      }
      else if (!command.isEmpty())
      {
        //imageDesc = WorkbenchImages.getImageDescriptorFromProgram(
        //    command, 0);
      }
    }
    this->VerifyImage();
  }

  return imageDesc;
}

void EditorDescriptor::VerifyImage() const
{
  //TODO Editor verify image
  //    if (!imageDesc) {
  //      imageDesc = WorkbenchImages
  //            .getImageDescriptor(ISharedImages.IMG_OBJ_FILE);
  //    }
  //    else {
  //      Image img = imageDesc.createImage(false);
  //      if (img == null) {
  //          // @issue what should be the default image?
  //          imageDesc = WorkbenchImages
  //                  .getImageDescriptor(ISharedImages.IMG_OBJ_FILE);
  //      } else {
  //          img.dispose();
  //      }
  //    }
}

QString EditorDescriptor::GetImageFilename() const
{
  if (!configurationElement)
  {
    return imageFilename;
  }
  return configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_ICON);
}

QString EditorDescriptor::GetCase() const
{
  if (configurationElement.IsNull())
  {
    return 0;
  }
  return configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_CASE);
}

QString EditorDescriptor::GetLabel() const
{
  //if (program == null)
  //{
  if (configurationElement.IsNull())
  {
    return editorName;
  }
  return configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME);
  //}
  //return program.getName();
}

QString EditorDescriptor::GetLauncher() const
{
  if (configurationElement.IsNull())
  {
    return launcherName;
  }
  return configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_LAUNCHER);
}

QString EditorDescriptor::GetPluginID() const
{
  if (!configurationElement.IsNull())
  {
    return configurationElement->GetContributor()->GetName();
  }
  return pluginIdentifier;
}

bool EditorDescriptor::IsInternal() const
{
  return this->GetOpenMode() == OPEN_INTERNAL;
}

bool EditorDescriptor::IsOpenInPlace() const
{
  return this->GetOpenMode() == OPEN_INPLACE;
}

bool EditorDescriptor::IsOpenExternal() const
{
  return this->GetOpenMode() == OPEN_EXTERNAL;
}

bool EditorDescriptor::LoadValues(IMemento::Pointer  /*memento*/)
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

void EditorDescriptor::SaveValues(IMemento::Pointer  /*memento*/)
{
  //  memento.putString(IWorkbenchConstants.TAG_LABEL, getLabel());
  //  memento.putString(IWorkbenchConstants.TAG_IMAGE, getImageFilename());
  //  memento.putString(IWorkbenchConstants.TAG_CLASS, getEditorClassName());
  //  memento.putString(IWorkbenchConstants.TAG_LAUNCHER, getLauncher());
  //  memento.putString(IWorkbenchConstants.TAG_FILE, getFileName());
  //  memento.putString(IWorkbenchConstants.TAG_ID, getId());
  //  memento.putString(IWorkbenchConstants.TAG_PLUGIN, getPluginId());
  //
  //  memento.putInteger(IWorkbenchConstants.TAG_OPEN_MODE, getOpenMode());
  //  // legacy: handle the older attribute names, needed to allow reading of workspace by pre-3.0-RCP
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

int EditorDescriptor::GetOpenMode() const
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

void EditorDescriptor::SetClassName(const QString& newClassName)
{
  className = newClassName;
}

void EditorDescriptor::SetConfigurationElement(
    IConfigurationElement::Pointer newConfigurationElement)
{
  configurationElement = newConfigurationElement;
}

void EditorDescriptor::SetFileName(const QString& aFileName)
{
  fileName = aFileName;
}

void EditorDescriptor::SetID(const QString& anID)
{
  id = anID;
}

void EditorDescriptor::SetLauncher(const QString& newLauncher)
{
  launcherName = newLauncher;
}

void EditorDescriptor::SetName(const QString& newName)
{
  editorName = newName;
}

void EditorDescriptor::SetOpenMode(int mode)
{
  openMode = mode;
}

void EditorDescriptor::SetPluginIdentifier(const QString& anID)
{
  pluginIdentifier = anID;
}

QString EditorDescriptor::ToString() const
{
  return "EditorDescriptor(id=" + this->GetId() + ", label=" + this->GetLabel() + ")";
}

QString EditorDescriptor::GetLocalId() const
{
  return this->GetId();
}

QString EditorDescriptor::GetPluginId() const
{
  return this->GetPluginID();
}

IEditorMatchingStrategy::Pointer EditorDescriptor::GetEditorMatchingStrategy()
{
  if (matchingStrategy.IsNull() && !matchingStrategyChecked)
  {
    matchingStrategyChecked = true;
    if (/*program == null &&*/!configurationElement.IsNull())
    {
      QString strategy = configurationElement->GetAttribute(
                           WorkbenchRegistryConstants::ATT_MATCHING_STRATEGY);
      if (!strategy.isEmpty())
      {
        try
        {
          matchingStrategy = configurationElement->CreateExecutableExtension<
              IEditorMatchingStrategy> (
              WorkbenchRegistryConstants::ATT_MATCHING_STRATEGY);
        }
        catch (const CoreException& e)
        {
          WorkbenchPlugin::Log(
              "Error creating editor management policy for editor id "
                + this->GetId(), e);
        }
      }
    }
  }
  return matchingStrategy;
}

}

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

#include "cherryPerspectiveDescriptor.h"

#include "cherryWorkbenchRegistryConstants.h"
#include "cherryWorkbenchPlugin.h"
#include "cherryPerspectiveRegistry.h"

namespace cherry
{

PerspectiveDescriptor::PerspectiveDescriptor(const std::string& id,
    const std::string& label, PerspectiveDescriptor::Pointer originalDescriptor)
{
  this->id = id;
  this->label = label;
  if (originalDescriptor != 0)
  {
    this->originalId = originalDescriptor->GetOriginalId();
    //this.image = originalDescriptor.image;

    // This perspective is based on a perspective in some bundle -- if
    // that
    // bundle goes away then I think it makes sense to treat this
    // perspective
    // the same as any other -- so store it with the original
    // descriptor's
    // bundle's list.
    //
    // It might also make sense the other way...removing the following
    // line
    // will allow the perspective to stay around when the originating
    // bundle
    // is unloaded.
    //
    // This might also have an impact on upgrade cases -- should we
    // really be
    // destroying all user customized perspectives when the older
    // version is
    // removed?
    //
    // I'm leaving this here for now since its a good example, but
    // wouldn't be
    // surprised if we ultimately decide on the opposite.
    //
    // The reason this line is important is that this is the value used
    // to
    // put the object into the UI level registry. When that bundle goes
    // away,
    // the registry will remove the entire list of objects. So if this
    // desc
    // has been put into that list -- it will go away.
    this->pluginId = originalDescriptor->GetPluginId();
  }
}

PerspectiveDescriptor::PerspectiveDescriptor(const std::string& id,
    IConfigurationElement::Pointer configElement)
{
  this->configElement = configElement;
  this->id = id;
  // Sanity check.
  if ((this->GetId() == "") || (this->GetLabel() == "") || (this->GetClassName() == ""))
  {
    throw CoreException(
        "Invalid extension (missing label, id or class name): " + this->GetId());
  }
}

IPerspectiveFactory::Pointer PerspectiveDescriptor::CreateFactory()
{
  // if there is an originalId, then use that descriptor instead
  if (originalId != "")
  {
    // Get the original descriptor to create the factory. If the
    // original is gone then nothing can be done.
    IPerspectiveDescriptor::Pointer target = dynamic_cast<PerspectiveRegistry*>(WorkbenchPlugin
        ::GetDefault()->GetPerspectiveRegistry())
    ->FindPerspectiveWithId(originalId);

    return target == 0 ? IPerspectiveFactory::Pointer(0) : target.Cast<PerspectiveDescriptor>()->CreateFactory();
  }

  // otherwise try to create the executable extension
  if (configElement != 0)
  {
    try
    {
      IPerspectiveFactory::Pointer factory(configElement
      ->CreateExecutableExtension<IPerspectiveFactory>(WorkbenchRegistryConstants::ATT_CLASS));
      return factory;
    }
    catch (CoreException& e)
    {
      // do nothing
    }
  }

  return IPerspectiveFactory::Pointer(0);
}

void PerspectiveDescriptor::DeleteCustomDefinition()
{
  dynamic_cast<PerspectiveRegistry*>(WorkbenchPlugin::GetDefault()
      ->GetPerspectiveRegistry())->DeleteCustomDefinition(PerspectiveDescriptor::Pointer(this));
}

std::string PerspectiveDescriptor::GetDescription()
{
  return configElement == 0 ? description : RegistryReader
  ::GetDescription(configElement);
}

bool PerspectiveDescriptor::GetFixed()
{
  if (configElement == 0) return fixed;

  bool val = false;
  configElement->GetBoolAttribute(WorkbenchRegistryConstants::ATT_FIXED, val);
  return val;
}

std::string PerspectiveDescriptor::GetId()
{
  return id;
}

std::string PerspectiveDescriptor::GetPluginId()
{
  return configElement == 0 ? pluginId : configElement->GetContributor();
}

std::string PerspectiveDescriptor::GetLabel()
{
  if (configElement == 0) return label;

  std::string val;
  configElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME, val);
  return val;
}

std::string PerspectiveDescriptor::GetOriginalId()
{
  if (originalId == "")
  {
    return this->GetId();
  }
  return originalId;
}

bool PerspectiveDescriptor::HasCustomDefinition()
{
  return dynamic_cast<PerspectiveRegistry*>(WorkbenchPlugin::GetDefault()
      ->GetPerspectiveRegistry())->HasCustomDefinition(PerspectiveDescriptor::Pointer(this));
}

bool PerspectiveDescriptor::HasDefaultFlag()
{
  if (configElement == 0)
  {
    return false;
  }

  bool val = false;
  configElement->GetBoolAttribute(WorkbenchRegistryConstants::ATT_DEFAULT, val);
  return val;
}

bool PerspectiveDescriptor::IsPredefined()
{
  return this->GetClassName() != "" && configElement != 0;
}

bool PerspectiveDescriptor::IsSingleton()
{
  if (configElement == 0)
    return singleton;

  bool val = false;
  configElement->GetBoolAttribute(WorkbenchRegistryConstants::ATT_SINGLETON, val);
  return val;
}

bool PerspectiveDescriptor::RestoreState(IMemento::Pointer memento)
{
//  IMemento childMem = memento
//  .getChild(IWorkbenchConstants.TAG_DESCRIPTOR);
//  if (childMem != null)
//  {
//    id = childMem.getString(IWorkbenchConstants.TAG_ID);
//    originalId = childMem.getString(IWorkbenchConstants.TAG_DESCRIPTOR);
//    label = childMem.getString(IWorkbenchConstants.TAG_LABEL);
//    className = childMem.getString(IWorkbenchConstants.TAG_CLASS);
//    singleton
//        = (childMem.getInteger(IWorkbenchConstants.TAG_SINGLETON) != null);
//
//    // Find a descriptor in the registry.
//    IPerspectiveDescriptor descriptor = WorkbenchPlugin.getDefault()
//    .getPerspectiveRegistry().findPerspectiveWithId(getOriginalId());
//
//    if (descriptor != null)
//    {
//      // Copy the state from the registred descriptor.
//      image = descriptor.getImageDescriptor();
//    }
//  }
//  return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null); //$NON-NLS-1$
  return true;
}

void PerspectiveDescriptor::RevertToPredefined()
{
  if (this->IsPredefined())
  {
    this->DeleteCustomDefinition();
  }
}

bool PerspectiveDescriptor::SaveState(IMemento::Pointer memento)
{
//  IMemento childMem = memento
//  .createChild(IWorkbenchConstants.TAG_DESCRIPTOR);
//  childMem.putString(IWorkbenchConstants.TAG_ID, getId());
//  if (originalId != null)
//  {
//    childMem.putString(IWorkbenchConstants.TAG_DESCRIPTOR, originalId);
//  }
//  childMem.putString(IWorkbenchConstants.TAG_LABEL, getLabel());
//  childMem.putString(IWorkbenchConstants.TAG_CLASS, getClassName());
//  if (singleton)
//  {
//    childMem.putInteger(IWorkbenchConstants.TAG_SINGLETON, 1);
//  }
//  return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null); //$NON-NLS-1$
  return true;
}

IConfigurationElement::Pointer PerspectiveDescriptor::GetConfigElement()
{
  return configElement;
}

std::string PerspectiveDescriptor::GetClassName()
{
  return configElement == 0 ? className : RegistryReader
  ::GetClassValue(configElement, WorkbenchRegistryConstants::ATT_CLASS);
}

}

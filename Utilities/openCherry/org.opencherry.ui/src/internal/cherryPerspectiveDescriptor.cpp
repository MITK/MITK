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

namespace cherry
{

PerspectiveDescriptor::PerspectiveDescriptor(const std::string& id,
    const std::string& label, PerspectiveDescriptor::Pointer originalDescriptor)
{
  this.id = id;
  this.label = label;
  if (originalDescriptor != null)
  {
    this.originalId = originalDescriptor.getOriginalId();
    this.image = originalDescriptor.image;

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
    this.pluginId = originalDescriptor.getPluginId();
  }
}

PerspectiveDescriptor::PerspectiveDescriptor(const std::string& id,
    IConfigurationElement::Pointer configElement)
{
  this.configElement = configElement;
  this.id = id;
  // Sanity check.
  if ((getId() == null) || (getLabel() == null) || (getClassName() == null))
  {
    throw new CoreException(
        new Status(
            IStatus.ERROR,
            WorkbenchPlugin.PI_WORKBENCH,
            0,
            "Invalid extension (missing label, id or class name): " + getId(),//$NON-NLS-1$
            null));
  }
}

IPerspectiveFactory::Pointer CreateFactory()
{
  // if there is an originalId, then use that descriptor instead
  if (originalId != null)
  {
    // Get the original descriptor to create the factory. If the
    // original is gone then nothing can be done.
    IPerspectiveDescriptor target = ((PerspectiveRegistry) WorkbenchPlugin
        .getDefault().getPerspectiveRegistry())
    .findPerspectiveWithId(originalId);

    return target == null ? null : ((PerspectiveDescriptor) target)
    .createFactory();
  }

  // otherwise try to create the executable extension
  if (configElement != null)
  {
    try
    {
      return (IPerspectiveFactory) configElement
      .createExecutableExtension(IWorkbenchRegistryConstants.ATT_CLASS);
    }
    catch (CoreException e)
    {
      // do nothing
    }
  }

  return null;
}

void PerspectiveDescriptor::DeleteCustomDefinition()
{
  ((PerspectiveRegistry) WorkbenchPlugin.getDefault()
      .getPerspectiveRegistry()).deleteCustomDefinition(this);
}

std::string PerspectiveDescriptor::GetDescription()
{
  return configElement == null ? description : RegistryReader
  .getDescription(configElement);
}

bool PerspectiveDescriptor::GetFixed()
{
  return configElement == null ? fixed : Boolean.valueOf(
      configElement
      .getAttribute(IWorkbenchRegistryConstants.ATT_FIXED))
  .booleanValue();
}

std::string PerspectiveDescriptor::GetId()
{
  return id;
}

std::string PerspectiveDescriptor::GetLabel()
{
  return configElement == null ? label : configElement
  .getAttribute(IWorkbenchRegistryConstants.ATT_NAME);
}

std::string PerspectiveDescriptor::GetOriginalId()
{
  if (originalId == null)
  {
    return getId();
  }
  return originalId;
}

bool PerspectiveDescriptor::HasCustomDefinition()
{
  return ((PerspectiveRegistry) WorkbenchPlugin.getDefault()
      .getPerspectiveRegistry()).hasCustomDefinition(this);
}

bool PerspectiveDescriptor::HasDefaultFlag()
{
  if (configElement == null)
  {
    return false;
  }

  return Boolean.valueOf(
      configElement
      .getAttribute(IWorkbenchRegistryConstants.ATT_DEFAULT))
  .booleanValue();
}

bool PerspectiveDescriptor::IsPredefined()
{
  return getClassName() != null && configElement != null;
}

bool PerspectiveDescriptor::IsSingleton()
{
  return configElement == null ? singleton : configElement
  .getAttributeAsIs(IWorkbenchRegistryConstants.ATT_SINGLETON) != null;
}

bool PerspectiveDescriptor::RestoreState(IMemento::Pointer memento)
{
  IMemento childMem = memento
  .getChild(IWorkbenchConstants.TAG_DESCRIPTOR);
  if (childMem != null)
  {
    id = childMem.getString(IWorkbenchConstants.TAG_ID);
    originalId = childMem.getString(IWorkbenchConstants.TAG_DESCRIPTOR);
    label = childMem.getString(IWorkbenchConstants.TAG_LABEL);
    className = childMem.getString(IWorkbenchConstants.TAG_CLASS);
    singleton
        = (childMem.getInteger(IWorkbenchConstants.TAG_SINGLETON) != null);

    // Find a descriptor in the registry.
    IPerspectiveDescriptor descriptor = WorkbenchPlugin.getDefault()
    .getPerspectiveRegistry().findPerspectiveWithId(getOriginalId());

    if (descriptor != null)
    {
      // Copy the state from the registred descriptor.
      image = descriptor.getImageDescriptor();
    }
  }
  return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null); //$NON-NLS-1$
}

void PerspectiveDescriptor::RevertToPredefined()
{
  if (isPredefined())
  {
    deleteCustomDefinition();
  }
}

bool PerspectiveDescriptor::SaveState(IMemento::Pointer memento)
{
  IMemento childMem = memento
  .createChild(IWorkbenchConstants.TAG_DESCRIPTOR);
  childMem.putString(IWorkbenchConstants.TAG_ID, getId());
  if (originalId != null)
  {
    childMem.putString(IWorkbenchConstants.TAG_DESCRIPTOR, originalId);
  }
  childMem.putString(IWorkbenchConstants.TAG_LABEL, getLabel());
  childMem.putString(IWorkbenchConstants.TAG_CLASS, getClassName());
  if (singleton)
  {
    childMem.putInteger(IWorkbenchConstants.TAG_SINGLETON, 1);
  }
  return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null); //$NON-NLS-1$
}

IConfigurationElement::Pointer PerspectiveDescriptor::GetConfigElement()
{
  return configElement;
}

std::string PerspectiveDescriptor::GetClassName()
{
  return configElement == null ? className : RegistryReader
  .getClassValue(configElement, IWorkbenchRegistryConstants.ATT_CLASS);
}

}

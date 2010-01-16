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

#include "berryPerspectiveDescriptor.h"

#include "berryWorkbenchRegistryConstants.h"
#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchConstants.h"
#include "berryPerspectiveRegistry.h"

namespace berry
{

PerspectiveDescriptor::PerspectiveDescriptor(const std::string& id,
    const std::string& label, PerspectiveDescriptor::Pointer originalDescriptor)
 : singleton(false), fixed(false)
{
  this->id = id;
  this->label = label;
  if (originalDescriptor != 0)
  {
    this->originalId = originalDescriptor->GetOriginalId();
    this->imageDescriptor = originalDescriptor->imageDescriptor;

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
 : singleton(false), fixed(false)
{
  this->configElement = configElement;
  this->id = id;
  // Sanity check.
  if ((this->GetId() == "") || (this->GetLabel() == "")
      || (this->GetFactoryClassName() == ""))
  {
    throw CoreException("Invalid extension (missing label, id or class name): "
        + this->GetId());
  }
}

IPerspectiveFactory::Pointer PerspectiveDescriptor::CreateFactory()
{
  // if there is an originalId, then use that descriptor instead
  if (originalId != "")
  {
    // Get the original descriptor to create the factory. If the
    // original is gone then nothing can be done.
    IPerspectiveDescriptor::Pointer
        target =
            dynamic_cast<PerspectiveRegistry*> (WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry()) ->FindPerspectiveWithId(
                originalId);

    return target == 0 ? IPerspectiveFactory::Pointer(0) : target.Cast<
        PerspectiveDescriptor> ()->CreateFactory();
  }

  // otherwise try to create the executable extension
  if (configElement != 0)
  {
    try
    {
      IPerspectiveFactory::Pointer factory(
          configElement ->CreateExecutableExtension<IPerspectiveFactory> (
              WorkbenchRegistryConstants::ATT_CLASS));
      return factory;
    } catch (CoreException& /*e*/)
    {
      // do nothing
    }
  }

  return IPerspectiveFactory::Pointer(0);
}

void PerspectiveDescriptor::DeleteCustomDefinition()
{
  dynamic_cast<PerspectiveRegistry*> (WorkbenchPlugin::GetDefault() ->GetPerspectiveRegistry())->DeleteCustomDefinition(
      PerspectiveDescriptor::Pointer(this));
}

std::string PerspectiveDescriptor::GetDescription() const
{
  return configElement == 0 ? description : RegistryReader::GetDescription(
      configElement);
}

bool PerspectiveDescriptor::GetFixed() const
{
  if (configElement == 0)
    return fixed;

  bool val = false;
  configElement->GetBoolAttribute(WorkbenchRegistryConstants::ATT_FIXED, val);
  return val;
}

std::string PerspectiveDescriptor::GetId() const
{
  return id;
}

std::string PerspectiveDescriptor::GetPluginId() const
{
  return configElement == 0 ? pluginId : configElement->GetContributor();
}

ImageDescriptor::Pointer PerspectiveDescriptor::GetImageDescriptor() const
{
  if (imageDescriptor)
    return imageDescriptor;

  if (configElement)
  {
    std::string icon;
    configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ICON, icon);
    if (!icon.empty())
    {
      imageDescriptor = AbstractUIPlugin::ImageDescriptorFromPlugin(
          configElement->GetContributor(), icon);
    }

  }

  if (!imageDescriptor)
  {
    imageDescriptor = ImageDescriptor::GetMissingImageDescriptor();
  }

  return imageDescriptor;
}

std::string PerspectiveDescriptor::GetLabel() const
{
  if (configElement == 0)
    return label;

  std::string val;
  configElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME, val);
  return val;
}

std::string PerspectiveDescriptor::GetOriginalId() const
{
  if (originalId == "")
  {
    return this->GetId();
  }
  return originalId;
}

bool PerspectiveDescriptor::HasCustomDefinition() const
{
  return dynamic_cast<PerspectiveRegistry*> (WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry())->HasCustomDefinition(
      PerspectiveDescriptor::ConstPointer(this));
}

bool PerspectiveDescriptor::HasDefaultFlag() const
{
  if (configElement == 0)
  {
    return false;
  }

  bool val = false;
  configElement->GetBoolAttribute(WorkbenchRegistryConstants::ATT_DEFAULT, val);
  return val;
}

bool PerspectiveDescriptor::IsPredefined() const
{
  return this->GetFactoryClassName() != "" && configElement != 0;
}

bool PerspectiveDescriptor::IsSingleton() const
{
  if (configElement == 0)
    return singleton;

  bool val = false;
  configElement->GetBoolAttribute(WorkbenchRegistryConstants::ATT_SINGLETON,
      val);
  return val;
}

bool PerspectiveDescriptor::RestoreState(IMemento::Pointer memento)
{
  IMemento::Pointer childMem(memento->GetChild(
      WorkbenchConstants::TAG_DESCRIPTOR));
  if (childMem)
  {
    childMem->GetString(WorkbenchConstants::TAG_ID, id);
    childMem->GetString(WorkbenchConstants::TAG_DESCRIPTOR, originalId);
    childMem->GetString(WorkbenchConstants::TAG_LABEL, label);
    childMem->GetString(WorkbenchConstants::TAG_CLASS, className);
    int singletonVal;
    singleton = childMem->GetInteger(WorkbenchConstants::TAG_SINGLETON,
        singletonVal);

    // Find a descriptor in the registry.
    IPerspectiveDescriptor::Pointer
        descriptor =
            WorkbenchPlugin::GetDefault() ->GetPerspectiveRegistry()->FindPerspectiveWithId(
                this->GetOriginalId());

    if (descriptor)
    {
      // Copy the state from the registred descriptor.
      imageDescriptor = descriptor->GetImageDescriptor();
    }
  }
  //return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null); //$NON-NLS-1$
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
  IMemento::Pointer childMem(memento->CreateChild(
      WorkbenchConstants::TAG_DESCRIPTOR));
  childMem->PutString(WorkbenchConstants::TAG_ID, GetId());
  if (!originalId.empty())
  {
    childMem->PutString(WorkbenchConstants::TAG_DESCRIPTOR, originalId);
  }
  childMem->PutString(WorkbenchConstants::TAG_LABEL, GetLabel());
  childMem->PutString(WorkbenchConstants::TAG_CLASS, GetFactoryClassName());
  if (singleton)
  {
    childMem->PutInteger(WorkbenchConstants::TAG_SINGLETON, 1);
  }
  //return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null);
  return true;
}

IConfigurationElement::Pointer PerspectiveDescriptor::GetConfigElement() const
{
  return configElement;
}

std::string PerspectiveDescriptor::GetFactoryClassName() const
{
  return configElement == 0 ? className : RegistryReader::GetClassValue(
      configElement, WorkbenchRegistryConstants::ATT_CLASS);
}

}

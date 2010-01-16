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

#include "berryIntroDescriptor.h"

#include "berryIntroConstants.h"
#include "../berryWorkbenchRegistryConstants.h"

#include <berryAbstractUIPlugin.h>

namespace berry
{

IntroDescriptor::IntroDescriptor(IConfigurationElement::Pointer configElement)
    throw (CoreException) :
  element(configElement)
{
  std::string val;
  if (!configElement->GetAttribute(WorkbenchRegistryConstants::ATT_CLASS, val))
  {
    //TODO IStatus
    /*
    throw CoreException(new Status(IStatus.ERROR,
        configElement .getNamespace(), 0,
        "Invalid extension (Missing class name): " + getId(), //$NON-NLS-1$
        null));
        */
    throw CoreException(configElement->GetContributor() + ": Invalid extension (Missing className): " + GetId());
  }
}

SmartPointer<IIntroPart> IntroDescriptor::CreateIntro() throw (CoreException)
{
  IIntroPart::Pointer intro(element->CreateExecutableExtension<IIntroPart>(
      WorkbenchRegistryConstants::ATT_CLASS));
  return intro;
}

IntroContentDetector::Pointer IntroDescriptor::GetIntroContentDetector()
    throw (CoreException)
{
  std::string val;
  if (!element->GetAttribute(WorkbenchRegistryConstants::ATT_CONTENT_DETECTOR, val))
  {
    return IntroContentDetector::Pointer(0);
  }

  IntroContentDetector::Pointer detector(
    element->CreateExecutableExtension<IntroContentDetector>(
      WorkbenchRegistryConstants::ATT_CONTENT_DETECTOR));
  return detector;
}

int IntroDescriptor::GetRole() const
{
  std::string role;
  if (!element->GetAttribute(WorkbenchRegistryConstants::ATT_ROLE, role))
  {
    return IntroConstants::INTRO_ROLE_VIEW;
  }

  if (role == "editor") return IntroConstants::INTRO_ROLE_EDITOR;
  else return IntroConstants::INTRO_ROLE_VIEW;
}

std::string IntroDescriptor::GetId() const
{
  std::string id;
  element->GetAttribute(WorkbenchRegistryConstants::ATT_ID, id);
  return id;
}

std::string IntroDescriptor::GetPluginId() const
{
  return element->GetContributor();
}

ImageDescriptor::Pointer IntroDescriptor::GetImageDescriptor() const
{
  if (imageDescriptor)
  {
    return imageDescriptor;
  }
  std::string iconName;
  if (!element->GetAttribute(WorkbenchRegistryConstants::ATT_ICON, iconName))
  {
    return ImageDescriptor::Pointer();
  }

  imageDescriptor = AbstractUIPlugin::ImageDescriptorFromPlugin(
      element->GetContributor(), iconName);
  return imageDescriptor;
}

IConfigurationElement::Pointer IntroDescriptor::GetConfigurationElement() const
{
  return element;
}

std::string IntroDescriptor::GetLabelOverride() const
{
  std::string label;
  element->GetAttribute(WorkbenchRegistryConstants::ATT_LABEL, label);
  return label;
}

}

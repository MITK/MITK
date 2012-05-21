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

#include "berryIntroDescriptor.h"

#include "berryIntroConstants.h"
#include "internal/berryWorkbenchRegistryConstants.h"

#include <berryAbstractUICTKPlugin.h>

namespace berry
{

IntroDescriptor::IntroDescriptor(IConfigurationElement::Pointer configElement)
    throw (CoreException) :
  element(configElement)
{
  QString val = configElement->GetAttribute(WorkbenchRegistryConstants::ATT_CLASS);
  if (val.isEmpty())
  {
    //TODO IStatus
    /*
    throw CoreException(new Status(IStatus.ERROR,
        configElement .getNamespace(), 0,
        "Invalid extension (Missing class name): " + getId(), //$NON-NLS-1$
        null));
        */
    throw CoreException(configElement->GetContributor() +
                        ": Invalid extension (Missing className): " + GetId());
  }
}

SmartPointer<IIntroPart> IntroDescriptor::CreateIntro() throw (CoreException)
{
  IIntroPart::Pointer intro(element->CreateExecutableExtension<IIntroPart>(WorkbenchRegistryConstants::ATT_CLASS));
  return intro;
}

IntroContentDetector::Pointer IntroDescriptor::GetIntroContentDetector()
    throw (CoreException)
{
  QString val = element->GetAttribute(WorkbenchRegistryConstants::ATT_CONTENT_DETECTOR);
  if (val.isEmpty())
  {
    return IntroContentDetector::Pointer(0);
  }

  IntroContentDetector::Pointer detector(
    element->CreateExecutableExtension<IntroContentDetector>(WorkbenchRegistryConstants::ATT_CONTENT_DETECTOR));
  return detector;
}

int IntroDescriptor::GetRole() const
{
  QString role = element->GetAttribute(WorkbenchRegistryConstants::ATT_ROLE);
  if (role.isEmpty())
  {
    return IntroConstants::INTRO_ROLE_VIEW;
  }

  if (role == "editor") return IntroConstants::INTRO_ROLE_EDITOR;
  else return IntroConstants::INTRO_ROLE_VIEW;
}

QString IntroDescriptor::GetId() const
{
  return element->GetAttribute(WorkbenchRegistryConstants::ATT_ID);
}

QString IntroDescriptor::GetPluginId() const
{
  return element->GetContributor();
}

ImageDescriptor::Pointer IntroDescriptor::GetImageDescriptor() const
{
  if (imageDescriptor)
  {
    return imageDescriptor;
  }
  QString iconName = element->GetAttribute(WorkbenchRegistryConstants::ATT_ICON);
  if (iconName.isEmpty())
  {
    return ImageDescriptor::Pointer();
  }

  imageDescriptor = AbstractUICTKPlugin::ImageDescriptorFromPlugin(
        element->GetContributor(), iconName);
  return imageDescriptor;
}

IConfigurationElement::Pointer IntroDescriptor::GetConfigurationElement() const
{
  return element;
}

QString IntroDescriptor::GetLabelOverride() const
{
  return element->GetAttribute(WorkbenchRegistryConstants::ATT_LABEL);
}

}

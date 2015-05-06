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
#include <berryStatus.h>
#include <berryCoreException.h>
#include <berryIContributor.h>

namespace berry
{

IntroDescriptor::IntroDescriptor(IConfigurationElement::Pointer configElement)
  : element(configElement)
{
  QString val = configElement->GetAttribute(WorkbenchRegistryConstants::ATT_CLASS);
  if (val.isEmpty())
  {
    IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, configElement->GetContributor()->GetName(), 0,
                                       QString("Invalid extension (Missing class name): ") + GetId()));
    throw CoreException(status);
  }
}

SmartPointer<IIntroPart> IntroDescriptor::CreateIntro()
{
  IIntroPart::Pointer intro(element->CreateExecutableExtension<IIntroPart>(WorkbenchRegistryConstants::ATT_CLASS));
  return intro;
}

IntroContentDetector::Pointer IntroDescriptor::GetIntroContentDetector()
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
  return element->GetContributor()->GetName();
}

QIcon IntroDescriptor::GetImageDescriptor() const
{
  if (!imageDescriptor.isNull())
  {
    return imageDescriptor;
  }
  QString iconName = element->GetAttribute(WorkbenchRegistryConstants::ATT_ICON);
  if (iconName.isEmpty())
  {
    return QIcon();
  }

  return AbstractUICTKPlugin::ImageDescriptorFromPlugin(
        element->GetContributor()->GetName(), iconName);
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

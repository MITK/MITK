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

#include "berryIntroRegistry.h"

#include "berryIConfigurationElement.h"

#include "berryIntroDescriptor.h"
#include "../berryRegistryReader.h"
#include "../berryWorkbenchPlugin.h"

namespace berry
{

const std::string IntroRegistry::TAG_INTRO = "intro";
const std::string IntroRegistry::TAG_INTROPRODUCTBINDING = "introProductBinding";
const std::string IntroRegistry::ATT_INTROID = "introId";
const std::string IntroRegistry::ATT_PRODUCTID = "productId";

std::string IntroRegistry::GetIntroForProduct(
    const std::string& targetProductId,
    const std::vector<const IExtension*>& extensions) const
{
  for (std::size_t i = 0; i < extensions.size(); i++)
  {
    std::vector<IConfigurationElement::Pointer> elements(
        extensions[i] ->GetConfigurationElements());
    for (std::size_t j = 0; j < elements.size(); j++)
    {
      if (elements[j]->GetName() == TAG_INTROPRODUCTBINDING)
      {
        std::string introId;
        bool hasIntroId = elements[j]->GetAttribute(ATT_INTROID, introId);
        std::string productId;
        bool hasProductId = elements[j]->GetAttribute(ATT_PRODUCTID, productId);

        if (!hasIntroId || !hasProductId)
        {
          //TODO IStatus
          /*
           IStatus status = new Status(
           IStatus.ERROR,
           elements[j].getDeclaringExtension()
           .getNamespace(),
           IStatus.ERROR,
           "introId and productId must be defined.", new IllegalArgumentException()); //$NON-NLS-1$
           WorkbenchPlugin.log("Invalid intro binding", status); //$NON-NLS-1$
           */
          WorkbenchPlugin::Log(
              elements[j]->GetDeclaringExtension()->GetNamespace()
                  + ": Invalid intro binding. introId and productId must be defined");
          continue;
        }

        if (targetProductId == productId)
        {
          return introId;
        }
      }
    }
  }
  return "";
}

int IntroRegistry::GetIntroCount() const
{
  return static_cast<int> (GetIntros().size());
}

std::vector<IIntroDescriptor::Pointer> IntroRegistry::GetIntros() const
{
  const IExtensionPoint* point =
      Platform::GetExtensionPointService()->GetExtensionPoint(
          PlatformUI::PLUGIN_ID + "." + WorkbenchRegistryConstants::PL_INTRO);
  if (!point)
  {
    return std::vector<IIntroDescriptor::Pointer>();
  }

  std::vector<const IExtension*> extensions(point->GetExtensions());
  extensions = RegistryReader::OrderExtensions(extensions);

  std::vector<IIntroDescriptor::Pointer> list;
  for (std::size_t i = 0; i < extensions.size(); i++)
  {
    std::vector<IConfigurationElement::Pointer> elements(
        extensions[i] ->GetConfigurationElements());
    for (std::size_t j = 0; j < elements.size(); j++)
    {
      if (elements[j]->GetName() == TAG_INTRO)
      {
        try
        {
          IIntroDescriptor::Pointer
              descriptor(new IntroDescriptor(elements[j]));
          list.push_back(descriptor);
        } catch (CoreException& e)
        {
          // log an error since its not safe to open a dialog here
          //TODO IStatus
          WorkbenchPlugin::Log("Unable to create intro descriptor", e); // e.getStatus());
        }
      }
    }
  }

  return list;
}

IIntroDescriptor::Pointer IntroRegistry::GetIntroForProduct(
    const std::string& targetProductId) const
{
  const IExtensionPoint* point =
      Platform::GetExtensionPointService()->GetExtensionPoint(
          PlatformUI::PLUGIN_ID + "." + WorkbenchRegistryConstants::PL_INTRO);
  if (!point)
  {
    return IIntroDescriptor::Pointer();
  }

  std::vector<const IExtension*> extensions(point->GetExtensions());
  extensions = RegistryReader::OrderExtensions(extensions);

  std::string targetIntroId = GetIntroForProduct(targetProductId, extensions);
  if (targetIntroId.empty())
  {
    return IIntroDescriptor::Pointer();
  }

  IIntroDescriptor::Pointer descriptor;

  std::vector<IIntroDescriptor::Pointer> intros(GetIntros());
  for (std::size_t i = 0; i < intros.size(); i++)
  {
    if (intros[i]->GetId() == targetIntroId)
    {
      descriptor = intros[i];
      break;
    }
  }

  return descriptor;
}

IIntroDescriptor::Pointer IntroRegistry::GetIntro(const std::string& id) const
{
  std::vector<IIntroDescriptor::Pointer> intros(GetIntros());
  for (std::size_t i = 0; i < intros.size(); i++)
  {
    IIntroDescriptor::Pointer desc = intros[i];
    if (desc->GetId() == id)
    {
      return desc;
    }
  }
  return IIntroDescriptor::Pointer();
}

}

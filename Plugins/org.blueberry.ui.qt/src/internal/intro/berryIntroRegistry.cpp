/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIntroRegistry.h"

#include <berryIConfigurationElement.h>
#include <berryIExtension.h>

#include "berryIntroDescriptor.h"
#include "internal/berryRegistryReader.h"
#include "berryWorkbenchPlugin.h"

namespace berry
{

const QString IntroRegistry::TAG_INTRO = "intro";
const QString IntroRegistry::TAG_INTROPRODUCTBINDING = "introProductBinding";
const QString IntroRegistry::ATT_INTROID = "introId";
const QString IntroRegistry::ATT_PRODUCTID = "productId";

QString IntroRegistry::GetIntroForProduct(
    const QString& targetProductId,
    const QList<IExtension::Pointer>& extensions) const
{
  for (int i = 0; i < extensions.size(); i++)
  {
    QList<IConfigurationElement::Pointer> elements(
        extensions[i]->GetConfigurationElements());
    for (int j = 0; j < elements.size(); j++)
    {
      if (elements[j]->GetName() == TAG_INTROPRODUCTBINDING)
      {
        QString introId = elements[j]->GetAttribute(ATT_INTROID);
        QString productId = elements[j]->GetAttribute(ATT_PRODUCTID);

        if (introId.isEmpty() || productId.isEmpty())
        {
          //TODO IStatus
          /*
           IStatus status = new Status(
           IStatus.ERROR,
           elements[j].getDeclaringExtension()
           .getNamespace(),
           IStatus.ERROR,
           "introId and productId must be defined.", new IllegalArgumentException());
           WorkbenchPlugin.log("Invalid intro binding", status);
           */
          WorkbenchPlugin::Log(
                elements[j]->GetDeclaringExtension()->GetNamespaceIdentifier()
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

QList<IIntroDescriptor::Pointer> IntroRegistry::GetIntros() const
{
  IExtensionPoint::Pointer point =
      Platform::GetExtensionRegistry()->GetExtensionPoint(
        PlatformUI::PLUGIN_ID() + "." + WorkbenchRegistryConstants::PL_INTRO);
  if (!point)
  {
    return QList<IIntroDescriptor::Pointer>();
  }

  QList<IExtension::Pointer> extensions(point->GetExtensions());
  extensions = RegistryReader::OrderExtensions(extensions);

  QList<IIntroDescriptor::Pointer> list;
  for (int i = 0; i < extensions.size(); i++)
  {
    QList<IConfigurationElement::Pointer> elements(
        extensions[i]->GetConfigurationElements());
    for (int j = 0; j < elements.size(); j++)
    {
      if (elements[j]->GetName() == TAG_INTRO)
      {
        try
        {
          IIntroDescriptor::Pointer
              descriptor(new IntroDescriptor(elements[j]));
          list.push_back(descriptor);
        }
        catch (const CoreException& e)
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
    const QString& targetProductId) const
{
  IExtensionPoint::Pointer point =
      Platform::GetExtensionRegistry()->GetExtensionPoint(
        PlatformUI::PLUGIN_ID() + "." + WorkbenchRegistryConstants::PL_INTRO);
  if (!point)
  {
    return IIntroDescriptor::Pointer();
  }

  QList<IExtension::Pointer> extensions(point->GetExtensions());
  extensions = RegistryReader::OrderExtensions(extensions);

  QString targetIntroId = GetIntroForProduct(targetProductId, extensions);
  if (targetIntroId.isEmpty())
  {
    return IIntroDescriptor::Pointer();
  }

  IIntroDescriptor::Pointer descriptor;

  QList<IIntroDescriptor::Pointer> intros(GetIntros());
  for (int i = 0; i < intros.size(); i++)
  {
    if (intros[i]->GetId() == targetIntroId)
    {
      descriptor = intros[i];
      break;
    }
  }

  return descriptor;
}

IIntroDescriptor::Pointer IntroRegistry::GetIntro(const QString& id) const
{
  QList<IIntroDescriptor::Pointer> intros(GetIntros());
  for (int i = 0; i < intros.size(); i++)
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

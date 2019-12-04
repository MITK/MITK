/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryKeywordRegistry.h"

#include "berryIConfigurationElement.h"
#include "berryIExtension.h"
#include "berryIExtensionPoint.h"
#include "berryIExtensionPointFilter.h"
#include "berryIExtensionRegistry.h"

#include "berryExtensionTracker.h"
#include "berryObjectString.h"
#include "berryPlatform.h"
#include "berryPlatformUI.h"
#include "berryWorkbenchRegistryConstants.h"

namespace berry {

const QString KeywordRegistry::ATT_ID = "id";
const QString KeywordRegistry::ATT_LABEL = "label";
const QString KeywordRegistry::TAG_KEYWORD = "keyword";

KeywordRegistry::KeywordRegistry()
{
  IExtensionTracker* tracker = PlatformUI::GetWorkbench()->GetExtensionTracker();
  tracker->RegisterHandler(this, ExtensionTracker::CreateExtensionPointFilter(GetExtensionPointFilter()));
  QList<IExtension::Pointer> extensions = GetExtensionPointFilter()->GetExtensions();
  for (auto& extension : extensions)
  {
    AddExtension(tracker, extension);
  }
}

SmartPointer<IExtensionPoint> KeywordRegistry::GetExtensionPointFilter() const
{
  return Platform::GetExtensionRegistry()->GetExtensionPoint(
        PlatformUI::PLUGIN_ID(), WorkbenchRegistryConstants::PL_KEYWORDS);
}

KeywordRegistry*KeywordRegistry::GetInstance()
{
  static KeywordRegistry instance;
  return &instance;
}

void KeywordRegistry::AddExtension(IExtensionTracker* tracker, const SmartPointer<IExtension>& extension)
{
  for (auto element : extension->GetConfigurationElements())
  {
    if (element->GetName() == TAG_KEYWORD)
    {
      QString name = element->GetAttribute(ATT_LABEL);
      QString id = element->GetAttribute(ATT_ID);
      internalKeywordMap.insert(id, name);
      Object::Pointer trackedObject(new ObjectString(id));
      tracker->RegisterObject(extension, trackedObject, IExtensionTracker::REF_STRONG);
    }
  }
}

QString KeywordRegistry::GetKeywordLabel(const QString& id)
{
  auto it = internalKeywordMap.find(id);
  return it == internalKeywordMap.end() ? QString::null : *it;
}

void KeywordRegistry::RemoveExtension(const SmartPointer<IExtension>& /*extension*/, const QList<SmartPointer<Object> >& objects)
{
  for (auto object : objects)
  {
    if (ObjectString::Pointer objString = object.Cast<ObjectString>())
    {
      internalKeywordMap.remove(*objString);
    }
  }
}

}

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMTagsOfInterestService.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>

#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyPersistence.h>
#include <mitkTemporoSpatialStringProperty.h>

mitk::IPropertyDescriptions*
GetDescriptionsService()
{
  mitk::IPropertyDescriptions* result = nullptr;

  std::vector<us::ServiceReference<mitk::IPropertyDescriptions> > descriptionRegisters = us::GetModuleContext()->GetServiceReferences<mitk::IPropertyDescriptions>();
  if (!descriptionRegisters.empty())
  {
    if (descriptionRegisters.size() > 1)
    {
      MITK_WARN << "Multiple property description services found. Using just one.";
    }
    result = us::GetModuleContext()->GetService<mitk::IPropertyDescriptions>(descriptionRegisters.front());
  }

  return result;
};

mitk::IPropertyPersistence*
GetPersistenceService()
{
  mitk::IPropertyPersistence* result = nullptr;

  std::vector<us::ServiceReference<mitk::IPropertyPersistence> > persRegisters = us::GetModuleContext()->GetServiceReferences<mitk::IPropertyPersistence>();
  if (!persRegisters.empty())
  {
    if (persRegisters.size() > 1)
    {
      MITK_WARN << "Multiple property description services found. Using just one.";
    }
    result = us::GetModuleContext()->GetService<mitk::IPropertyPersistence>(persRegisters.front());
  }

  return result;
};

mitk::DICOMTagsOfInterestService::
DICOMTagsOfInterestService()
{
};

mitk::DICOMTagsOfInterestService::
~DICOMTagsOfInterestService()
{
};

void
mitk::DICOMTagsOfInterestService::
RegisterDescriptionAndPersistence(const DICOMTagPath& tagPath, bool makePersistant)
{
  std::string propRegEx = mitk::DICOMTagPathToPropertyRegEx(tagPath);

  mitk::IPropertyDescriptions* descriptionSrv = GetDescriptionsService();
  if (descriptionSrv)
  {
    descriptionSrv->AddDescriptionRegEx(propRegEx, "DICOM tag: " + tagPath.GetLastNode().tag.GetName());
  }

  mitk::IPropertyPersistence* persSrv = GetPersistenceService();
  if (persSrv && makePersistant)
  {
    PropertyPersistenceInfo::Pointer info = PropertyPersistenceInfo::New();
    if (tagPath.IsExplicit())
    {
      std::string name = mitk::DICOMTagPathToPropertyName(tagPath);
      std::string key = name;
      std::replace(key.begin(), key.end(), '.', '_');
      info->SetNameAndKey(name, key);
    }
    else
    {
      std::string key = mitk::DICOMTagPathToPersistenceKeyRegEx(tagPath);
      std::string keyTemplate = mitk::DICOMTagPathToPersistenceKeyTemplate(tagPath);
      std::string propTemplate = mitk::DICOMTagPathToPersistenceNameTemplate(tagPath);
      info->UseRegEx(propRegEx, propTemplate, key, keyTemplate);
    }

    info->SetDeserializationFunction(mitk::PropertyPersistenceDeserialization::deserializeJSONToTemporoSpatialStringProperty);
    info->SetSerializationFunction(mitk::PropertyPersistenceSerialization::serializeTemporoSpatialStringPropertyToJSON);
    persSrv->AddInfo(info);
  }
};

void
mitk::DICOMTagsOfInterestService::
AddTagOfInterest(const DICOMTagPath& tagPath, bool makePersistant)
{
  if (tagPath.Size() == 0)
  {
    MITK_DEBUG << "Indication for wrong DICOMTagsOfInterestService::AddTagOfInterest() usage. Empty DICOM tag path was passed.";
    return;
  }

  MutexHolder lock(m_Lock);
  // Only the scanned form belongs in m_Tags: the derived path would make the
  // scanner hunt a functional-group macro at the top level of every file.
  this->m_Tags[tagPath] = makePersistant;

  this->RegisterDescriptionAndPersistence(tagPath, makePersistant);

  if (mitk::IsFunctionalGroupRooted(tagPath))
  {
    // The reader publishes a functional-group finding under its frame-relative
    // key, so without this the property matches no persistence info and
    // ItkImageIO drops it on save without a log line. The rooted form stays
    // registered: a file whose per-frame item count does not match its frame
    // count keeps the one-frame model and publishes rooted keys.
    this->RegisterDescriptionAndPersistence(mitk::FunctionalGroupRelativePath(tagPath), makePersistant);
  }
};

mitk::DICOMTagPathMapType
mitk::DICOMTagsOfInterestService::
GetTagsOfInterest() const
{
  MutexHolder lock(m_Lock);
  DICOMTagPathMapType result;

  for (const auto& tag : this->m_Tags)
  {
    result.insert(std::make_pair(tag.first, ""));
  }

  return result;
};

bool
mitk::DICOMTagsOfInterestService::
HasTag(const DICOMTagPath& tag) const
{
  return this->m_Tags.find(tag) != this->m_Tags.cend();
};

void
mitk::DICOMTagsOfInterestService::
UnregisterDescriptionAndPersistence(const DICOMTagPath& tagPath)
{
  std::string propRegEx = mitk::DICOMTagPathToPropertyRegEx(tagPath);

  mitk::IPropertyDescriptions* descriptionSrv = GetDescriptionsService();
  if (descriptionSrv)
  {
    descriptionSrv->RemoveDescription(propRegEx);
  }

  mitk::IPropertyPersistence* persSrv = GetPersistenceService();
  if (persSrv)
  {
    persSrv->RemoveInfo(propRegEx);
  }
};

void
mitk::DICOMTagsOfInterestService::
RestoreKeyIfStillWanted(const DICOMTagPath& publishedKey)
{
  // Several tags can publish under one key: the shared and the per-frame root of
  // one attribute both derive to it, and it can be a tag of interest in its own
  // right. Removing one of them must leave what the others asked for standing,
  // with their persistence rather than a guess. Persistence is the union,
  // because one holder needing it is enough.
  bool stillWanted = false;
  bool stillPersistent = false;

  for (const auto& remaining : this->m_Tags)
  {
    const bool publishesUnderKey =
      remaining.first == publishedKey
      || (mitk::IsFunctionalGroupRooted(remaining.first)
          && mitk::FunctionalGroupRelativePath(remaining.first) == publishedKey);

    if (publishesUnderKey)
    {
      stillWanted = true;
      stillPersistent = stillPersistent || remaining.second;
    }
  }

  if (stillWanted)
  {
    this->RegisterDescriptionAndPersistence(publishedKey, stillPersistent);
  }
};

void
mitk::DICOMTagsOfInterestService::
RemoveTag(const DICOMTagPath& tag)
{
  MutexHolder lock(m_Lock);
  this->m_Tags.erase(tag);
  this->UnregisterDescriptionAndPersistence(tag);

  // The removed path can itself be the key a functional-group tag publishes
  // under, so its registration is restored on the same terms as any other.
  this->RestoreKeyIfStillWanted(tag);

  if (mitk::IsFunctionalGroupRooted(tag))
  {
    const DICOMTagPath derived = mitk::FunctionalGroupRelativePath(tag);
    this->UnregisterDescriptionAndPersistence(derived);
    this->RestoreKeyIfStillWanted(derived);
  }
};

void
mitk::DICOMTagsOfInterestService::
RemoveAllTags()
{
  MutexHolder lock(m_Lock);

  for (const auto& tag : m_Tags)
  {
    this->UnregisterDescriptionAndPersistence(tag.first);

    if (mitk::IsFunctionalGroupRooted(tag.first))
    {
      this->UnregisterDescriptionAndPersistence(mitk::FunctionalGroupRelativePath(tag.first));
    }
  }

  this->m_Tags.clear();
};

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMTagsOfInterestService.h"

#include "usModuleContext.h"
#include "usGetModuleContext.h"

#include "mitkIPropertyDescriptions.h"
#include "mitkIPropertyPersistence.h"
#include "mitkTemporoSpatialStringProperty.h"

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
AddTagOfInterest(const DICOMTagPath& tagPath, bool makePersistant)
{
  if (tagPath.Size() == 0)
  {
    MITK_DEBUG << "Indication for wrong DICOMTagsOfInterestService::AddTagOfInterest() usage. Empty DICOM tag path was passed.";
    return;
  }

  MutexHolder lock(m_Lock);
  std::string propRegEx = mitk::DICOMTagPathToPropertyRegEx(tagPath);
  this->m_Tags.insert(tagPath);

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

mitk::DICOMTagPathMapType
mitk::DICOMTagsOfInterestService::
GetTagsOfInterest() const
{
  MutexHolder lock(m_Lock);
  DICOMTagPathMapType result;

  for (auto tag : this->m_Tags)
  {
    result.insert(std::make_pair(tag, ""));
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
RemoveTag(const DICOMTagPath& tag)
{
  MutexHolder lock(m_Lock);
  this->m_Tags.erase(tag);
  std::string propRegEx = mitk::DICOMTagPathToPropertyRegEx(tag);

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
RemoveAllTags()
{
  MutexHolder lock(m_Lock);
  mitk::IPropertyDescriptions* descriptionSrv = GetDescriptionsService();
  mitk::IPropertyPersistence* persSrv = GetPersistenceService();

  for (const auto& tag : m_Tags)
  {
    std::string propRegEx = mitk::DICOMTagPathToPropertyRegEx(tag);

    if (descriptionSrv)
    {
      descriptionSrv->RemoveDescription(propRegEx);
    }

    if (persSrv)
    {
      persSrv->RemoveInfo(propRegEx);
    }
  }

  this->m_Tags.clear();
};

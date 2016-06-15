/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkDICOMTagsOfInterestService.h"
#include "mitkDICOMTagHelper.h"

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
AddTagOfInterest(const DICOMTag& tag, bool makePersistant)
{
  MutexHolder lock(m_Lock);
  std::string propName = mitk::GeneratPropertyNameForDICOMTag(tag);
  this->m_TagMap.insert(std::make_pair(propName, tag));
  this->m_PersMap[propName] = makePersistant; //this must be changed even if the propname already exists.
  this->m_KnownTags.insert(propName);

  mitk::IPropertyDescriptions* descriptionSrv = GetDescriptionsService();
  if (descriptionSrv)
  {
    descriptionSrv->AddDescription(propName, "DICOM tag: " + tag.GetName());
  }

  mitk::IPropertyPersistence* persSrv = GetPersistenceService();
  if (persSrv)
  {
    std::string key = propName;
    std::replace(key.begin(), key.end(), '.', '_');

    PropertyPersistenceInfo::Pointer info = PropertyPersistenceInfo::New(key);
    info->SetDeserializationFunction(mitk::PropertyPersistenceDeserialization::deserializeJSONToTemporoSpatialStringProperty);
    info->SetSerializationFunction(mitk::PropertyPersistenceSerialization::serializeTemporoSpatialStringPropertyToJSON);
    persSrv->AddInfo(propName, info);
  }
};

mitk::DICOMTagsOfInterestService::DICOMTagMapType
mitk::DICOMTagsOfInterestService::
GetTagsOfInterest() const
{
  MutexHolder lock(m_Lock);
  DICOMTagMapType result;

  for (auto tag : this->m_TagMap)
  {
    InternalTagSetType::const_iterator finding = this->m_KnownTags.find(tag.first);
    if (finding == this->m_KnownTags.cend())
    {
      mitkThrow() << "Invalid status. Tag is missing in the known tag set. Problematic tag:" << tag.first;
    }

    result.insert(std::make_pair(finding->c_str(), tag.second));
  }

  return result;
};

bool
mitk::DICOMTagsOfInterestService::
HasTag(const DICOMTag& tag) const
{
  std::string propName = mitk::GeneratPropertyNameForDICOMTag(tag);
  return this->m_TagMap.find(propName) != this->m_TagMap.cend();
};

void
mitk::DICOMTagsOfInterestService::
RemoveTag(const DICOMTag& tag)
{
  MutexHolder lock(m_Lock);
  std::string propName = mitk::GeneratPropertyNameForDICOMTag(tag);
  this->m_PersMap.erase(propName);
  this->m_TagMap.erase(propName);

  mitk::IPropertyDescriptions* descriptionSrv = GetDescriptionsService();
  if (descriptionSrv && descriptionSrv->HasDescription(propName))
  {
    descriptionSrv->RemoveDescription(propName);
  }

  mitk::IPropertyPersistence* persSrv = GetPersistenceService();
  if (persSrv && persSrv->HasInfos(propName))
  {
    persSrv->RemoveInfos(propName);
  }
};

void
mitk::DICOMTagsOfInterestService::
RemoveAllTags()
{
  MutexHolder lock(m_Lock);
  this->m_PersMap.clear();
  this->m_TagMap.clear();
};

/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <regex>
#include <mutex>

#include "mitkSourceImageRelationRule.h"
#include "mitkPropertyNameHelper.h"
#include "mitkStringProperty.h"
#include "mitkTemporoSpatialStringProperty.h"
#include "mitkDataNode.h"
#include "mitkIdentifiable.h"

std::string mitk::SourceImageRelationRule::GenerateRuleID(const std::string& purpose) const
{
  std::string result = "SourceImageRelation";
  if (!purpose.empty())
  {
    result += " " + purpose;
  }
  return result;
}

bool mitk::SourceImageRelationRule::IsAbstract() const
{
  return m_PurposeTag.empty();
};

bool mitk::SourceImageRelationRule::IsSupportedRuleID(const RuleIDType& ruleID) const
{
  return ruleID == this->GetRuleID() || (IsAbstract() && ruleID.find("SourceImageRelation ") == 0);
};

mitk::SourceImageRelationRule::RuleIDType mitk::SourceImageRelationRule::GetRuleID() const
{
  return this->GenerateRuleID(m_PurposeTag);
};

std::string mitk::SourceImageRelationRule::GetDisplayName() const
{
  return m_DisplayName;
};

std::string mitk::SourceImageRelationRule::GetSourceRoleName() const
{
  return m_SourceRole;
};

std::string mitk::SourceImageRelationRule::GetDestinationRoleName() const
{
  return m_DestinationRole;
};

bool mitk::SourceImageRelationRule::IsDestinationCandidate(const IPropertyProvider *owner) const
{
  auto node = dynamic_cast<const DataNode*>(owner);

  auto image = nullptr != node
    ? dynamic_cast<const Image*>(node->GetData())
    : dynamic_cast<const Image*>(owner);

  return image != nullptr;
}

mitk::SourceImageRelationRule::RelationUIDType mitk::SourceImageRelationRule::Connect(Image *source, const Image *destination) const
{
  return Superclass::Connect(source, destination);
};

mitk::SourceImageRelationRule::SourceImageRelationRule()
  : m_PurposeTag(""), m_DisplayName("Abstract image to image relation"), m_SourceRole("derived data"), m_DestinationRole("source image")
{};

mitk::SourceImageRelationRule::SourceImageRelationRule(const RuleIDType &purposeTag)
  : SourceImageRelationRule(purposeTag, purposeTag + " relation"){};

mitk::SourceImageRelationRule::SourceImageRelationRule(const RuleIDType &purposeTag, const std::string &displayName)
  : SourceImageRelationRule(
      purposeTag, displayName, "derived data", "source image"){};

mitk::SourceImageRelationRule::SourceImageRelationRule(const RuleIDType &purposeTag,
                                                   const std::string &displayName,
                                                   const std::string &sourceRole,
                                                   const std::string &destinationRole)
  : m_PurposeTag(purposeTag), m_DisplayName(displayName), m_SourceRole(sourceRole), m_DestinationRole(destinationRole){};

mitk::SourceImageRelationRule::DataRelationUIDVectorType
mitk::SourceImageRelationRule::GetRelationUIDs_DataLayer(const IPropertyProvider* source,
  const IPropertyProvider* destination, const InstanceIDVectorType& instances_IDLayer) const
{
  DataRelationUIDVectorType result;

  auto relevantIndicesAndRuleIDs = GetReferenceSequenceIndices(source, destination, instances_IDLayer);

  auto itemRIIRegExStr = this->GetRIIPropertyRegEx("SourceImageSequenceItem");
  auto regEx = std::regex(itemRIIRegExStr);

  //workaround until T24729 is done. Please remove if T24728 is done
  auto keys = PropertyRelationRuleBase::GetPropertyKeys(source);
  //end workaround for T24729

  for (const auto &indexNRule : relevantIndicesAndRuleIDs)
  {
    bool relationCoveredByRII = false;
    for (const auto& key : keys)
    {
      if (std::regex_match(key, regEx))
      {
        auto sequItemProp = source->GetConstProperty(key);
        if (sequItemProp.IsNotNull() && sequItemProp->GetValueAsString() == std::to_string(indexNRule.first))
        {
          relationCoveredByRII = true;
          auto instanceID = GetInstanceIDByPropertyName(key);
          auto ruleID = GetRuleIDByInstanceID(source, instanceID);
          if (this->IsSupportedRuleID(ruleID))
          {
            result.emplace_back(this->GetRelationUIDByInstanceID(source, instanceID), ruleID);
          }
        }
      }
    }

    if (!relationCoveredByRII)
    {
      //the relation is not covered on the RII level, so we generate the property path to the DICOM source reference (this is done via
      //the SOP Instance UIDs which uniquely identify an DICOM IOD). We use this property path as relation UID because it is identifying
      //on the data level (even so not long term stable if relations with a lower index are removed).
      PropertyKeyPath referencedInstanceUIDs;
      referencedInstanceUIDs.AddElement("DICOM").AddElement("0008").AddSelection("2112", indexNRule.first).AddElement("0008").AddElement("1155");

      std::string ruleID = "";
      if (!indexNRule.second.empty())
      {
        ruleID = this->GenerateRuleID(indexNRule.second);
      }

      result.emplace_back(PropertyKeyPathToPropertyName(referencedInstanceUIDs), ruleID);
    }
  }

  return result;
};

std::vector<std::pair<size_t,std::string> > mitk::SourceImageRelationRule::GetReferenceSequenceIndices(const IPropertyProvider * source,
  const IPropertyProvider * destination, InstanceIDVectorType ignoreInstances) const
{
  std::vector<std::pair<size_t, std::string> > result;

  BaseProperty::ConstPointer destInstanceUIDProp;
  std::string destinationUID = "";

  if (destination)
  {
    destInstanceUIDProp = destination->GetConstProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x0018));

    if (destInstanceUIDProp.IsNull())
    {
      return result;
    }

    auto identifiable = this->CastProviderAsIdentifiable(destination);

    if (identifiable)
    {
      destinationUID= identifiable->GetUID();
    }
  }

  std::vector<std::string> ignoreItemIndices;
  for (const auto& iID : ignoreInstances)
  {
    auto sourceImageRefPath = GetRootKeyPath().AddElement(iID).AddElement("SourceImageSequenceItem");
    auto imageRefProp = source->GetConstProperty(PropertyKeyPathToPropertyName(sourceImageRefPath));

    if (imageRefProp.IsNotNull())
    {
      ignoreItemIndices.emplace_back(imageRefProp->GetValueAsString());
    }
  }

  PropertyKeyPath referencedInstanceUIDs;
  referencedInstanceUIDs.AddElement("DICOM").AddElement("0008").AddAnySelection("2112").AddElement("0008").AddElement("1155");

  auto sourceRegExStr = PropertyKeyPathToPropertyRegEx(referencedInstanceUIDs);
  auto regEx = std::regex(sourceRegExStr);

  std::vector<std::string> keys;
  //workaround until T24729 is done. Please remove if T24728 is done
  keys = PropertyRelationRuleBase::GetPropertyKeys(source);
  //end workaround for T24729

  for (const auto &key : keys)
  {
    if (std::regex_match(key, regEx))
    {
      auto refUIDProp = source->GetConstProperty(key);
      if (destination==nullptr || *refUIDProp == *destInstanceUIDProp)
      {
        auto currentKeyPath = PropertyNameToPropertyKeyPath(key);
        auto currentKeyPathSelection = currentKeyPath.GetNode(2).selection;

        auto finding = std::find(ignoreItemIndices.begin(), ignoreItemIndices.end(), std::to_string(currentKeyPathSelection));
        if (finding == ignoreItemIndices.end())
        {
          PropertyKeyPath purposePath;
          purposePath.AddElement("DICOM").AddElement("0008").AddSelection("2112", currentKeyPathSelection).AddElement("0040").AddSelection("a170", 0).AddElement("0008").AddElement("0104");
          auto purposeProp = source->GetConstProperty(PropertyKeyPathToPropertyName(purposePath));
          std::string currentPurpose = "";
          if (purposeProp.IsNotNull())
          {
            currentPurpose = purposeProp->GetValueAsString();
          }
          if (this->IsAbstract() || (purposeProp.IsNotNull() && currentPurpose == this->m_PurposeTag))
          {
            result.emplace_back(currentKeyPathSelection, currentPurpose);
          }
        }
      }
    }
  }

  return result;
};

/**This mutex is used to guard mitk::SourceImageRelationRule::CreateNewSourceImageSequenceItem by a class wide mutex to avoid
racing conditions in a scenario where rules are used concurrently. It is not in the class interface itself, because it
is an implementation detail.
*/
namespace
{
  std::mutex sequenceItemCreationLock;
}

mitk::PropertyKeyPath::ItemSelectionIndex mitk::SourceImageRelationRule::CreateNewSourceImageSequenceItem(
  IPropertyOwner *source) const
{
  std::lock_guard<std::mutex> guard(sequenceItemCreationLock);

  //////////////////////////////////////
  // Get all existing sequence items

  std::vector<PropertyKeyPath::ItemSelectionIndex> instanceIDs;
  PropertyKeyPath::ItemSelectionIndex newID = 0;

  PropertyKeyPath referencedInstanceUIDs;
  referencedInstanceUIDs.AddElement("DICOM").AddElement("0008").AddAnySelection("2112").AddElement("0008").AddElement("1155");
  auto regExStr = PropertyKeyPathToPropertyRegEx(referencedInstanceUIDs);
  auto regEx = std::regex(regExStr);
  std::smatch instance_matches;

  //workaround until T24729 is done. You can use directly source->GetPropertyKeys again, when fixed.
  const auto keys = GetPropertyKeys(source);
  //end workaround for T24729

  for (const auto &key : keys)
  {
    if (std::regex_search(key, instance_matches, regEx))
    {
      if (instance_matches.size()>1)
      {
        instanceIDs.push_back(std::stoi(instance_matches[1]));
      }
    }
  }

  //////////////////////////////////////
  // Get new ID

  std::sort(instanceIDs.begin(), instanceIDs.end());
  if (!instanceIDs.empty())
  {
    newID = instanceIDs.back()+1;
  }

  //////////////////////////////////////
  // reserve new ID

  PropertyKeyPath newSourceImageSequencePath;
  newSourceImageSequencePath.AddElement("DICOM").AddElement("0008").AddSelection("2112",newID).AddElement("0008").AddElement("1155");

  auto newKey =
    PropertyKeyPathToPropertyName(newSourceImageSequencePath);
  source->SetProperty(newKey, mitk::TemporoSpatialStringProperty::New("reserved slot for source image sequence"));

  return newID;
};


void mitk::SourceImageRelationRule::Connect_datalayer(IPropertyOwner * source,
                                                    const IPropertyProvider * destination,
                                                    const InstanceIDType & instanceID) const
{
  auto destInstanceUIDProp = destination->GetConstProperty(GeneratePropertyNameForDICOMTag(0x0008,0x0018));
  auto destClassUIDProp = destination->GetConstProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x0016));

  if (destInstanceUIDProp.IsNotNull() && destClassUIDProp.IsNotNull())
  {
    auto existingRefs = this->GetReferenceSequenceIndices(source, destination);
    std::string  newSelectionIndexStr;
    if (!existingRefs.empty())
    {
      newSelectionIndexStr = std::to_string(existingRefs[0].first);
    }
    else
    {
      PropertyKeyPath::ItemSelectionIndex newSelectionIndex = CreateNewSourceImageSequenceItem(source);

      PropertyKeyPath refInstanceUIDPath;
      refInstanceUIDPath.AddElement("DICOM").AddElement("0008").AddSelection("2112", newSelectionIndex).AddElement("0008").AddElement("1155");
      source->SetProperty(PropertyKeyPathToPropertyName(refInstanceUIDPath), destInstanceUIDProp->Clone());

      PropertyKeyPath refClassUIDPath;
      refClassUIDPath.AddElement("DICOM").AddElement("0008").AddSelection("2112", newSelectionIndex).AddElement("0008").AddElement("1150");
      source->SetProperty(PropertyKeyPathToPropertyName(refClassUIDPath), destClassUIDProp->Clone());

      PropertyKeyPath purposePath;
      purposePath.AddElement("DICOM").AddElement("0008").AddSelection("2112", newSelectionIndex).AddElement("0040").AddSelection("a170", 0).AddElement("0008").AddElement("0104");
      source->SetProperty(PropertyKeyPathToPropertyName(purposePath), StringProperty::New(m_PurposeTag));

      newSelectionIndexStr = std::to_string(newSelectionIndex);
    }

    auto sourceImageRefPath = GetRootKeyPath().AddElement(instanceID).AddElement("SourceImageSequenceItem");
    source->SetProperty(PropertyKeyPathToPropertyName(sourceImageRefPath), StringProperty::New(newSelectionIndexStr).GetPointer());
  }
  else
  {
    MITK_DEBUG << "Cannot connect SourceImageRelationRule on data layer. Passed destination does not have properties for DICOM SOP Instance UIDs(0x0008, 0x0018) and DICOM SOP Class UID(0x0008, 0x0016)";
  }
};

void mitk::SourceImageRelationRule::Disconnect_datalayer(IPropertyOwner * source, const RelationUIDType & relationUID) const
{
  std::string deletedImageRefSequenceIndexStr = "";

  if (relationUID.find("DICOM") == 0)
  { //relation that is purly data based.
    auto currentKeyPath = PropertyNameToPropertyKeyPath(relationUID);
    deletedImageRefSequenceIndexStr = std::to_string(currentKeyPath.GetNode(2).selection);
  }
  else
  {
    auto sourceImageRefPath = GetRootKeyPath().AddElement(this->GetInstanceIDByRelationUID(source,relationUID)).AddElement("SourceImageSequenceItem");
    auto imageRefProp = source->GetConstProperty(PropertyKeyPathToPropertyName(sourceImageRefPath));
    if (imageRefProp.IsNotNull())
    {
      deletedImageRefSequenceIndexStr = imageRefProp->GetValueAsString();
    }
  }

  if(!deletedImageRefSequenceIndexStr.empty())
  {
    auto deletedImageRefSequenceIndex = std::stoull(deletedImageRefSequenceIndexStr);
    auto refs = GetReferenceSequenceIndices(source);
    std::sort(refs.begin(), refs.end());

    for (const auto &refIndex : refs)
    {
      if (refIndex.first >= deletedImageRefSequenceIndex)
      {
        PropertyKeyPath refDICOMDataPath;
        refDICOMDataPath.AddElement("DICOM").AddElement("0008").AddSelection("2112", refIndex.first);
        auto prefix = PropertyKeyPathToPropertyName(refDICOMDataPath);

        PropertyKeyPath refRelDataPath = GetRootKeyPath().AddAnyElement().AddElement("SourceImageSequenceItem");;
        auto riiRegEx = std::regex(PropertyKeyPathToPropertyRegEx(refRelDataPath));

        //workaround until T24729 is done. You can use directly source->GetPropertyKeys again, when fixed.
        const auto keys = GetPropertyKeys(source);
        //end workaround for T24729

        for (const auto &key : keys)
        {
          if (key.find(prefix) == 0)
          { //its a relevant DICOM property delete or update
            if (refIndex.first != deletedImageRefSequenceIndex)
            {
              //reindex to close the gap in the dicom sequence.
              auto newPath = PropertyNameToPropertyKeyPath(key);
              newPath.GetNode(2).selection = refIndex.first - 1;
              source->SetProperty(PropertyKeyPathToPropertyName(newPath), source->GetNonConstProperty(key));
            }
            //remove old/outdated data layer information
            source->RemoveProperty(key);
          }
          if (std::regex_match(key, riiRegEx))
          { //it is a relevant RII property, remove it or update it.
            auto imageSequenceItemProp = source->GetConstProperty(key);
            if (imageSequenceItemProp->GetValueAsString() == deletedImageRefSequenceIndexStr)
            {
              source->RemoveProperty(key);
            }
            else if (imageSequenceItemProp->GetValueAsString() == std::to_string(refIndex.first))
            {
              //its a relevant data property of the relation rule reindex it.
              source->SetProperty(key, StringProperty::New(std::to_string(refIndex.first - 1)));
            }
          }
        }
      }
    }
  }
};

itk::LightObject::Pointer mitk::SourceImageRelationRule::InternalClone() const
{
  itk::LightObject::Pointer result = Self::New(this->m_PurposeTag, this->m_DisplayName, this->m_SourceRole, this->m_DestinationRole).GetPointer();

  return result;
};

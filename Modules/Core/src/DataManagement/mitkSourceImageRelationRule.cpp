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

#include <regex>
#include <mutex>

#include "mitkSourceImageRelationRule.h"
#include "mitkPropertyNameHelper.h"
#include "mitkStringProperty.h"
#include "mitkTemporoSpatialStringProperty.h"
#include "mitkDataNode.h"


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
  return "SourceImageRelation " + m_PurposeTag;
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

mitk::SourceImageRelationRule::InstanceIDVectorType mitk::SourceImageRelationRule::GetInstanceID_datalayer(
  const IPropertyProvider * source, const IPropertyProvider * destination) const
{
  InstanceIDVectorType result;

  auto relevantReferenceIndices = GetReferenceSequenceIndices(source, destination);

  auto itemRegExStr = this->GetRIIPropertyRegEx("SourceImageSequenceItem");
  auto regEx = std::regex(itemRegExStr);

  //workaround until T24729 is done. Please remove if T24728 is done
  auto keys = PropertyRelationRuleBase::GetPropertyKeys(source);
  //end workaround for T24729

  for (const auto &key : keys)
  {
    if (std::regex_match(key, regEx))
    {
      auto sequItemProp = source->GetConstProperty(key);
      if (sequItemProp.IsNotNull())
      {
        auto finding = std::find(std::cbegin(relevantReferenceIndices), std::cend(relevantReferenceIndices), sequItemProp->GetValueAsString());
        if (finding != std::cend(relevantReferenceIndices))
        {
          auto instanceID = GetInstanceIDByPropertyName(key);
          auto ruleID = GetRuleIDByInstanceID(source, instanceID);
          if (this->IsSupportedRuleID(ruleID))
          {
            result.push_back(instanceID);
          }
        }
      }
    }
  }

  return result;
};

std::vector<std::string> mitk::SourceImageRelationRule::GetReferenceSequenceIndices(const IPropertyProvider * source,
  const IPropertyProvider * destination) const
{
  std::vector<std::string> result;

  BaseProperty::ConstPointer destInstanceUIDProp;

  if (destination)
  {
    destInstanceUIDProp = destination->GetConstProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x0018));

    if (destInstanceUIDProp.IsNull())
    {
      return result;
    }
  }

  PropertyKeyPath referencedInstanceUIDs;
  referencedInstanceUIDs.AddElement("DICOM").AddElement("0008").AddAnySelection("2112").AddElement("0008").AddElement("1155");

  auto sourceRegExStr = PropertyKeyPathToPropertyRegEx(referencedInstanceUIDs);;
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
        PropertyKeyPath purposePath;
        purposePath.AddElement("DICOM").AddElement("0008").AddSelection("2112", currentKeyPathSelection).AddElement("0040").AddSelection("a170", 0).AddElement("0008").AddElement("0104");

        auto purposeProp = source->GetConstProperty(PropertyKeyPathToPropertyName(purposePath));
        if (this->IsAbstract() || (purposeProp.IsNotNull() && purposeProp->GetValueAsString() == this->m_PurposeTag))
        {
          result.push_back(std::to_string(currentKeyPathSelection));
        }
      }
    }
  }

  return result;
};

bool mitk::SourceImageRelationRule::HasImplicitDataRelation(const IPropertyProvider * source,
                                                          const IPropertyProvider * destination) const
{
  auto relevantReferences = GetReferenceSequenceIndices(source, destination);

  if (this->IsAbstract())
  {
    return !relevantReferences.empty();
  }
  else
  {
    for (auto referenceIndex : relevantReferences)
    {
      PropertyKeyPath purposePath;
      purposePath.AddElement("DICOM").AddElement("0008").AddSelection("2112", std::stoi(referenceIndex)).AddElement("0040").AddSelection("a170", 0).AddElement("0008").AddElement("0104");
      auto purposeProp = source->GetConstProperty(PropertyKeyPathToPropertyName(purposePath));

      if (purposeProp.IsNotNull() && purposeProp->GetValueAsString() == this->m_PurposeTag)
      {
        return true;
      }
    }
  }
  
  return false;
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
      newSelectionIndexStr = existingRefs[0];
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

void mitk::SourceImageRelationRule::Disconnect_datalayer(IPropertyOwner * source, const InstanceIDType & instanceID) const
{
  auto sourceImageRefPath = GetRootKeyPath().AddElement(instanceID).AddElement("SourceImageSequenceItem");
  auto imageRefProp = source->GetConstProperty(PropertyKeyPathToPropertyName(sourceImageRefPath));

  if (imageRefProp.IsNotNull())
  {
    auto deletedImageRefSequenceIndex = imageRefProp->GetValueAsString();

    auto refs = GetReferenceSequenceIndices(source);
    std::sort(refs.begin(), refs.end());

    for (auto refIndexStr : refs)
    {
      auto refIndex = std::stoi(refIndexStr);

      if (refIndex >= std::stoi(deletedImageRefSequenceIndex))
      {
        PropertyKeyPath refDICOMDataPath;
        refDICOMDataPath.AddElement("DICOM").AddElement("0008").AddSelection("2112", refIndex);
        auto prefix = PropertyKeyPathToPropertyName(refDICOMDataPath);

        PropertyKeyPath refRelDataPath = GetRootKeyPath().AddAnyElement().AddElement("SourceImageSequenceItem");;
        auto regEx = std::regex(PropertyKeyPathToPropertyRegEx(refRelDataPath));

        //workaround until T24729 is done. You can use directly source->GetPropertyKeys again, when fixed.
        const auto keys = GetPropertyKeys(source);
        //end workaround for T24729

        for (const auto &key : keys)
        {
          if (key.find(prefix) == 0)
          { //its a relevant DICOM property delete or update
            if (refIndexStr != deletedImageRefSequenceIndex)
            {
              //reindex to close the gap in the dicom sequence.
              auto newPath = PropertyNameToPropertyKeyPath(key);
              newPath.GetNode(2).selection = refIndex - 1;
              source->SetProperty(PropertyKeyPathToPropertyName(newPath), source->GetNonConstProperty(key));
            }
            //remove old/outdated data layer information
            source->RemoveProperty(key);

            auto sourceImageRefPath = GetRootKeyPath().AddElement(instanceID).AddElement("SourceImageSequenceItem");
          }
          if (std::regex_match(key, regEx))
          {  
            auto imageSequenceItemProp = source->GetConstProperty(key);
            if (imageSequenceItemProp->GetValueAsString() == std::to_string(refIndex))
            {
              //its a relevant data property of the relation rule.
              source->SetProperty(key, StringProperty::New(std::to_string(refIndex - 1)));
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

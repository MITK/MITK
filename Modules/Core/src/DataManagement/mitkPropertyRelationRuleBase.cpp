/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPropertyRelationRuleBase.h"

#include <mitkDataNode.h>
#include <mitkExceptionMacro.h>
#include <mitkNodePredicateBase.h>
#include <mitkStringProperty.h>
#include <mitkUIDGenerator.h>

#include <mutex>
#include <regex>
#include <algorithm>

bool mitk::PropertyRelationRuleBase::IsAbstract() const
{
  return true;
}

bool mitk::PropertyRelationRuleBase::IsSourceCandidate(const IPropertyProvider *owner) const
{
  return owner != nullptr;
}

bool mitk::PropertyRelationRuleBase::IsDestinationCandidate(const IPropertyProvider *owner) const
{
  return owner != nullptr;
}

mitk::PropertyKeyPath mitk::PropertyRelationRuleBase::GetRootKeyPath()
{
  return PropertyKeyPath().AddElement("MITK").AddElement("Relations");
}

bool mitk::PropertyRelationRuleBase::IsSupportedRuleID(const RuleIDType& ruleID) const
{
  return ruleID == this->GetRuleID();
}

mitk::PropertyKeyPath mitk::PropertyRelationRuleBase::GetRIIPropertyKeyPath(const std::string propName, const InstanceIDType& instanceID)
{
  auto path = GetRootKeyPath();
  if (instanceID.empty())
  {
    path.AddAnyElement();
  }
  else
  {
    path.AddElement(instanceID);
  }

  if (!propName.empty())
  {
    path.AddElement(propName);
  }

  return path;
}

std::string mitk::PropertyRelationRuleBase::GetRIIPropertyRegEx(const std::string propName, const InstanceIDType &instanceID) const
{
  return PropertyKeyPathToPropertyRegEx(GetRIIPropertyKeyPath(propName, instanceID));
}

mitk::PropertyKeyPath mitk::PropertyRelationRuleBase::GetRIIRelationUIDPropertyKeyPath(const InstanceIDType& instanceID)
{
  return GetRIIPropertyKeyPath("relationUID", instanceID);
}

mitk::PropertyKeyPath mitk::PropertyRelationRuleBase::GetRIIRuleIDPropertyKeyPath(const InstanceIDType& instanceID)
{
  return GetRIIPropertyKeyPath("ruleID", instanceID);
}

mitk::PropertyKeyPath mitk::PropertyRelationRuleBase::GetRIIDestinationUIDPropertyKeyPath(const InstanceIDType& instanceID)
{
  return GetRIIPropertyKeyPath("destinationUID", instanceID);
}

//workaround until T24729 is done. Please remove if T24728 is done
//then could directly use owner->GetPropertyKeys() again.
std::vector<std::string> mitk::PropertyRelationRuleBase::GetPropertyKeys(const mitk::IPropertyProvider *owner)
{
  std::vector<std::string> keys;
  auto sourceCasted = dynamic_cast<const mitk::DataNode*>(owner);
  if (sourceCasted) {
    auto sourceData = sourceCasted->GetData();
    if (sourceData) {
      keys = sourceData->GetPropertyKeys();
    }
    else {
      keys = sourceCasted->GetPropertyKeys();
    }
  }
  else {
    keys = owner->GetPropertyKeys();
  }
  return keys;
}
//end workaround for T24729

bool mitk::PropertyRelationRuleBase::IsSource(const IPropertyProvider *owner) const
{
  return !this->GetExistingRelations(owner).empty();
}

bool mitk::PropertyRelationRuleBase::HasRelation(
  const IPropertyProvider* source, const IPropertyProvider* destination, RelationType requiredRelation) const
{
  auto relTypes = this->GetRelationTypes(source, destination);


  if (requiredRelation == RelationType::None)
  {
    return !relTypes.empty();
  }

  RelationVectorType allowedTypes = { RelationType::Complete };
  if (requiredRelation == RelationType::Data)
  {
    allowedTypes.emplace_back(RelationType::Data);
  }
  else if (requiredRelation == RelationType::ID)
  {
    allowedTypes.emplace_back(RelationType::ID);
  }

  return relTypes.end() != std::find_first_of(relTypes.begin(), relTypes.end(), allowedTypes.begin(), allowedTypes.end());
}

mitk::PropertyRelationRuleBase::RelationVectorType mitk::PropertyRelationRuleBase::GetRelationTypes(
  const IPropertyProvider* source, const IPropertyProvider* destination) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }
  if (!destination)
  {
    mitkThrow() << "Error. Passed owner pointer is NULL";
  }

  auto instanceIDs_IDLayer = this->GetInstanceID_IDLayer(source, destination);
  auto relIDs_dataLayer = this->GetRelationUIDs_DataLayer(source, destination, {});
  if (relIDs_dataLayer.size() > 1)
  {
    MITK_WARN << "Property relation on data level is ambiguous. First relation is used. Relation UID: "
              << relIDs_dataLayer.front().first;
  }

  bool hasComplete = instanceIDs_IDLayer.end() != std::find_if(instanceIDs_IDLayer.begin(), instanceIDs_IDLayer.end(), [&](const InstanceIDVectorType::value_type& instanceID)
  {
    auto relID_IDlayer = this->GetRelationUIDByInstanceID(source, instanceID);
    auto ruleID_IDlayer = this->GetRuleIDByInstanceID(source, instanceID);

    return relIDs_dataLayer.end() != std::find_if(relIDs_dataLayer.begin(), relIDs_dataLayer.end(), [&](const DataRelationUIDVectorType::value_type& relID)
    {
      return relID.first == relID_IDlayer && relID.second == ruleID_IDlayer;
    });
  });

  bool hasID = instanceIDs_IDLayer.end() != std::find_if(instanceIDs_IDLayer.begin(), instanceIDs_IDLayer.end(), [&](const InstanceIDVectorType::value_type& instanceID)
  {
    auto relID_IDlayer = this->GetRelationUIDByInstanceID(source, instanceID);
    auto ruleID_IDlayer = this->GetRuleIDByInstanceID(source, instanceID);

    return relIDs_dataLayer.end() == std::find_if(relIDs_dataLayer.begin(), relIDs_dataLayer.end(), [&](const DataRelationUIDVectorType::value_type& relID)
    {
      return relID.first == relID_IDlayer && relID.second == ruleID_IDlayer;
    });
  });

  bool hasData = relIDs_dataLayer.end() != std::find_if(relIDs_dataLayer.begin(), relIDs_dataLayer.end(), [&](const DataRelationUIDVectorType::value_type& relID)
  {
    return instanceIDs_IDLayer.end() == std::find_if(instanceIDs_IDLayer.begin(), instanceIDs_IDLayer.end(), [&](const InstanceIDVectorType::value_type& instanceID)
    {
      auto relID_IDlayer = this->GetRelationUIDByInstanceID(source, instanceID);
      auto ruleID_IDlayer = this->GetRuleIDByInstanceID(source, instanceID);
      return relID.first == relID_IDlayer && relID.second == ruleID_IDlayer;
    });
  });

  RelationVectorType result;

  if (hasData)
  {
    result.emplace_back(RelationType::Data);
  }
  if (hasID)
  {
    result.emplace_back(RelationType::ID);
  }
  if (hasComplete)
  {
    result.emplace_back(RelationType::Complete);
  }

  return result;
}

mitk::PropertyRelationRuleBase::RelationUIDVectorType mitk::PropertyRelationRuleBase::GetExistingRelations(
  const IPropertyProvider *source, RelationType layer) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }

  RelationUIDVectorType relationUIDs;
  InstanceIDVectorType instanceIDs;

  if (layer != RelationType::Data)
  {
    auto ruleIDRegExStr = this->GetRIIPropertyRegEx("ruleID");
    auto regEx = std::regex(ruleIDRegExStr);

    //workaround until T24729 is done. You can use directly source->GetPropertyKeys again, when fixed.
    const auto keys = GetPropertyKeys(source);
    //end workaround for T24729

    for (const auto& key : keys)
    {
      if (std::regex_match(key, regEx))
      {
        auto idProp = source->GetConstProperty(key);
        auto ruleID = idProp->GetValueAsString();
        if (this->IsSupportedRuleID(ruleID))
        {
          auto instanceID = this->GetInstanceIDByPropertyName(key);
          instanceIDs.emplace_back(instanceID);
          relationUIDs.push_back(this->GetRelationUIDByInstanceID(source, instanceID));
        }
      }
    }
  }

  if (layer == RelationType::ID)
  {
    return relationUIDs;
  }

  DataRelationUIDVectorType relationUIDandRuleID_Data;
  if (layer != RelationType::ID)
  {
    relationUIDandRuleID_Data = this->GetRelationUIDs_DataLayer(source, nullptr, instanceIDs);
  }

  RelationUIDVectorType relationUIDs_Data;
  std::transform(relationUIDandRuleID_Data.begin(), relationUIDandRuleID_Data.end(), std::back_inserter(relationUIDs_Data),
    [](const DataRelationUIDVectorType::value_type& v) { return v.first; });

  if (layer == RelationType::Data)
  {
    return relationUIDs_Data;
  }

  std::sort(relationUIDs.begin(), relationUIDs.end());
  std::sort(relationUIDs_Data.begin(), relationUIDs_Data.end());

  RelationUIDVectorType result;

  if (layer == RelationType::Complete)
  {
    std::set_intersection(relationUIDs.begin(), relationUIDs.end(), relationUIDs_Data.begin(), relationUIDs_Data.end(), std::back_inserter(result));
  }
  else
  {
    std::set_union(relationUIDs.begin(), relationUIDs.end(), relationUIDs_Data.begin(), relationUIDs_Data.end(), std::back_inserter(result));
  }

  return result;
}

mitk::PropertyRelationRuleBase::RelationUIDVectorType mitk::PropertyRelationRuleBase::GetRelationUIDs(
  const IPropertyProvider *source, const IPropertyProvider *destination) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }
  if (!destination)
  {
    mitkThrow() << "Error. Passed destination pointer is NULL";
  }

  RelationUIDVectorType relUIDs_id;

  auto instanceIDs = this->GetInstanceID_IDLayer(source, destination);
  for (const auto& instanceID : instanceIDs)
  {
    relUIDs_id.push_back(this->GetRelationUIDByInstanceID(source, instanceID));
  }

  DataRelationUIDVectorType relationUIDandRuleID_Data = this->GetRelationUIDs_DataLayer(source,destination,instanceIDs);
  RelationUIDVectorType relUIDs_Data;
  std::transform(relationUIDandRuleID_Data.begin(), relationUIDandRuleID_Data.end(), std::back_inserter(relUIDs_Data),
    [](const DataRelationUIDVectorType::value_type& v) { return v.first; });

  std::sort(relUIDs_id.begin(), relUIDs_id.end());
  std::sort(relUIDs_Data.begin(), relUIDs_Data.end());

  RelationUIDVectorType result;
  std::set_union(relUIDs_id.begin(), relUIDs_id.end(), relUIDs_Data.begin(), relUIDs_Data.end(), std::back_inserter(result));

  return result;
}

mitk::PropertyRelationRuleBase::RelationUIDType
mitk::PropertyRelationRuleBase::GetRelationUID(const IPropertyProvider *source, const IPropertyProvider *destination) const
{
  auto result = this->GetRelationUIDs(source, destination);

  if (result.empty())
  {
    mitkThrowException(NoPropertyRelationException);
  }
  else if(result.size()>1)
  {
    mitkThrow() << "Cannot return one(!) relation UID. Multiple relations exists for given rule, source and destination.";
  }

  return result[0];
}

mitk::PropertyRelationRuleBase::InstanceIDType mitk::PropertyRelationRuleBase::NULL_INSTANCE_ID()
{
  return std::string();
};

mitk::PropertyRelationRuleBase::RelationUIDType mitk::PropertyRelationRuleBase::GetRelationUIDByInstanceID(
  const IPropertyProvider *source, const InstanceIDType &instanceID) const
{
  RelationUIDType result;

  if (instanceID != NULL_INSTANCE_ID())
  {
    auto idProp = source->GetConstProperty(
      PropertyKeyPathToPropertyName(GetRIIRelationUIDPropertyKeyPath(instanceID)));

    if (idProp.IsNotNull())
    {
      result = idProp->GetValueAsString();
    }
  }

  if (result.empty())
  {
    mitkThrowException(NoPropertyRelationException);
  }

  return result;
}

mitk::PropertyRelationRuleBase::InstanceIDType mitk::PropertyRelationRuleBase::GetInstanceIDByRelationUID(
  const IPropertyProvider *source, const RelationUIDType &relationUID) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }

  InstanceIDType result = NULL_INSTANCE_ID();

  auto destRegExStr =
    PropertyKeyPathToPropertyRegEx(GetRIIRelationUIDPropertyKeyPath());
  auto regEx = std::regex(destRegExStr);
  std::smatch instance_matches;

  //workaround until T24729 is done. You can use directly source->GetPropertyKeys again, when fixed.
  const auto keys = GetPropertyKeys(source);
  //end workaround for T24729

  for (const auto &key : keys)
  {
    if (std::regex_search(key, instance_matches, regEx))
    {
      auto idProp = source->GetConstProperty(key);
      if (idProp->GetValueAsString() == relationUID)
      {
        if (instance_matches.size()>1)
        {
          result = instance_matches[1];
          break;
        }
      }
    }
  }

  return result;
}

mitk::PropertyRelationRuleBase::InstanceIDVectorType mitk::PropertyRelationRuleBase::GetInstanceID_IDLayer(
  const IPropertyProvider *source, const IPropertyProvider *destination) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }
  if (!destination)
  {
    mitkThrow() << "Error. Passed destination pointer is NULL";
  }

  auto identifiable = CastProviderAsIdentifiable(destination);

  InstanceIDVectorType result;

  if (identifiable)
  { // check for relations of type Connected_ID;

    auto destRegExStr = this->GetRIIPropertyRegEx("destinationUID");
    auto regEx = std::regex(destRegExStr);
    std::smatch instance_matches;

    auto destUID = identifiable->GetUID();

    //workaround until T24729 is done. You can use directly source->GetPropertyKeys again, when fixed.
    const auto keys = GetPropertyKeys(source);
    //end workaround for T24729

    for (const auto &key : keys)
    {
      if (std::regex_search(key, instance_matches, regEx))
      {
        auto idProp = source->GetConstProperty(key);
        if (idProp->GetValueAsString() == destUID)
        {
          if (instance_matches.size()>1)
          {
            auto instanceID = instance_matches[1];
            if (this->IsSupportedRuleID(GetRuleIDByInstanceID(source, instanceID)))
            {
              result.push_back(instanceID);
            }
          }
        }
      }
    }
  }

  return result;
}

const mitk::Identifiable* mitk::PropertyRelationRuleBase::CastProviderAsIdentifiable(const mitk::IPropertyProvider* destination) const
{
  auto identifiable = dynamic_cast<const Identifiable*>(destination);

  if (!identifiable)
  { //This check and pass through to data is needed due to solve T25711. See Task for more information.
    //This could be removed at the point we can get rid of DataNodes or they get realy transparent.
    auto node = dynamic_cast<const DataNode*>(destination);
    if (node && node->GetData())
    {
      identifiable = dynamic_cast<const Identifiable*>(node->GetData());
    }
  }

  return identifiable;
}

mitk::PropertyRelationRuleBase::RelationUIDType mitk::PropertyRelationRuleBase::Connect(IPropertyOwner *source, const IPropertyProvider *destination) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }
  if (!destination)
  {
    mitkThrow() << "Error. Passed destination pointer is NULL";
  }
  if (this->IsAbstract())
  {
    mitkThrow() << "Error. This is an abstract property relation rule. Abstract rule must not make a connection. Please use a concrete rule.";
  }

  auto instanceIDs = this->GetInstanceID_IDLayer(source, destination);
  bool hasIDlayer = !instanceIDs.empty();

  auto relUIDs_data = this->GetRelationUIDs_DataLayer(source, destination, {});

  if (relUIDs_data.size() > 1)
  {
    MITK_WARN << "Property relation on data level is ambiguous. First relation is used. RelationUID ID: "
              << relUIDs_data.front().first;
  }

  bool hasDatalayer = !relUIDs_data.empty();

  RelationUIDType relationUID = this->CreateRelationUID();

  InstanceIDType instanceID = NULL_INSTANCE_ID();

  if (hasIDlayer)
  {
    instanceID = instanceIDs.front();
  }
  else if (hasDatalayer)
  {
    try
    {
      instanceID = this->GetInstanceIDByRelationUID(source, relUIDs_data.front().first);
    }
    catch(...)
    { }
  }

  if(instanceID == NULL_INSTANCE_ID())
  {
    instanceID = this->CreateNewRelationInstance(source, relationUID);
  }

  auto relUIDKey =
    PropertyKeyPathToPropertyName(GetRIIRelationUIDPropertyKeyPath(instanceID));
  source->SetProperty(relUIDKey, mitk::StringProperty::New(relationUID));

  auto ruleIDKey =
    PropertyKeyPathToPropertyName(GetRIIRuleIDPropertyKeyPath(instanceID));
  source->SetProperty(ruleIDKey, mitk::StringProperty::New(this->GetRuleID()));

  if (!hasIDlayer)
  {
    auto identifiable = this->CastProviderAsIdentifiable(destination);

    if (identifiable)
    {
      auto destUIDKey =
        PropertyKeyPathToPropertyName(GetRIIDestinationUIDPropertyKeyPath(instanceID));
      source->SetProperty(destUIDKey, mitk::StringProperty::New(identifiable->GetUID()));
    }
  }

  this->Connect_datalayer(source, destination, instanceID);

  return relationUID;
}

void mitk::PropertyRelationRuleBase::Disconnect(IPropertyOwner *source, const IPropertyProvider *destination, RelationType layer) const
{
  if (source == nullptr)
  {
    mitkThrow() << "Error. Source is invalid. Cannot disconnect.";
  }

  if (destination == nullptr)
  {
    mitkThrow() << "Error. Destination is invalid. Cannot disconnect.";
  }

  try
  {
    const auto relationUIDs = this->GetRelationUIDs(source, destination);
    for (const auto& relUID: relationUIDs)
    {
      this->Disconnect(source, relUID, layer);
    }
  }
  catch (const NoPropertyRelationException &)
  {
    // nothing to do and no real error in context of disconnect.
  }
}

void mitk::PropertyRelationRuleBase::Disconnect(IPropertyOwner *source, RelationUIDType relationUID, RelationType layer) const
{
  if (source == nullptr)
  {
    mitkThrow() << "Error. Source is invalid. Cannot disconnect.";
  }

  if (layer == RelationType::Data || layer == RelationType::Complete)
  {
    this->Disconnect_datalayer(source, relationUID);
  }

  auto instanceID = this->GetInstanceIDByRelationUID(source, relationUID);
  if ((layer == RelationType::ID || layer == RelationType::Complete) && instanceID != NULL_INSTANCE_ID())
  {
    auto instancePrefix = PropertyKeyPathToPropertyName(GetRootKeyPath().AddElement(instanceID));

    //workaround until T24729 is done. You can use directly source->GetPropertyKeys again, when fixed.
    const auto keys = GetPropertyKeys(source);
    //end workaround for T24729


    for (const auto &key : keys)
    {
      if (key.find(instancePrefix) == 0)
      {
        source->RemoveProperty(key);
      }
    }
  }
}

mitk::PropertyRelationRuleBase::RelationUIDType mitk::PropertyRelationRuleBase::CreateRelationUID()
{
  UIDGenerator generator;
  return generator.GetUID();
}

/**This mutex is used to guard mitk::PropertyRelationRuleBase::CreateNewRelationInstance by a class wide mutex to avoid
  racing conditions in a scenario where rules are used concurrently. It is not in the class interface itself, because it
  is an implementation detail.
*/
std::mutex relationCreationLock;

mitk::PropertyRelationRuleBase::InstanceIDType mitk::PropertyRelationRuleBase::CreateNewRelationInstance(
  IPropertyOwner *source, const RelationUIDType &relationUID) const
{
  std::lock_guard<std::mutex> guard(relationCreationLock);

  //////////////////////////////////////
  // Get all existing instanc IDs

  std::vector<int> instanceIDs;
  InstanceIDType newID = "1";

  auto destRegExStr =
    PropertyKeyPathToPropertyRegEx(GetRIIRelationUIDPropertyKeyPath());
  auto regEx = std::regex(destRegExStr);
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
    newID = std::to_string(instanceIDs.back() + 1);
  }

  //////////////////////////////////////
  // reserve new ID
  auto relUIDKey =
    PropertyKeyPathToPropertyName(GetRIIRelationUIDPropertyKeyPath(newID));
  source->SetProperty(relUIDKey, mitk::StringProperty::New(relationUID));

  return newID;
}

itk::LightObject::Pointer mitk::PropertyRelationRuleBase::InternalClone() const
{
  return Superclass::InternalClone();
}


mitk::PropertyRelationRuleBase::InstanceIDType mitk::PropertyRelationRuleBase::GetInstanceIDByPropertyName(const std::string propName)
{
  auto proppath = PropertyNameToPropertyKeyPath(propName);
  auto ref = GetRootKeyPath();

  if (proppath.GetSize() < 3 || !(proppath.GetFirstNode() == ref.GetFirstNode()) || !(proppath.GetNode(1) == ref.GetNode(1)))
  {
    mitkThrow() << "Property name is not for a RII property or containes no instance ID. Wrong name: " << propName;
  }

  return proppath.GetNode(2).name;
}

mitk::PropertyRelationRuleBase::RuleIDType mitk::PropertyRelationRuleBase::GetRuleIDByInstanceID(const IPropertyProvider *source,
  const InstanceIDType &instanceID) const
{
  if (!source)
  {
    mitkThrow() << "Error. Source is invalid. Cannot deduce rule ID";
  }

  auto path = GetRIIRuleIDPropertyKeyPath(instanceID);
  auto name = PropertyKeyPathToPropertyName(path);

  const auto prop = source->GetConstProperty(name);

  std::string result;

 if (prop.IsNotNull())
 {
   result = prop->GetValueAsString();
 }

  if (result.empty())
  {
    mitkThrowException(NoPropertyRelationException) << "Error. Source has no property relation with the passed instance ID. Instance ID: " << instanceID;
  }

  return result;
}

std::string mitk::PropertyRelationRuleBase::GetDestinationUIDByInstanceID(const IPropertyProvider* source,
  const InstanceIDType& instanceID) const
{
  if (!source)
  {
    mitkThrow() << "Error. Source is invalid. Cannot deduce rule ID";
  }

  auto path = GetRIIDestinationUIDPropertyKeyPath(instanceID);
  auto name = PropertyKeyPathToPropertyName(path);

  const auto prop = source->GetConstProperty(name);

  std::string result;

  if (prop.IsNotNull())
  {
    result = prop->GetValueAsString();
  }

  return result;
}


namespace mitk
{
  /**
  * \brief Predicate used to wrap rule checks.
  *
  * \ingroup DataStorage
  */
  class NodePredicateRuleFunction : public NodePredicateBase
  {
  public:
    using FunctionType = std::function<bool(const mitk::IPropertyProvider *, const mitk::PropertyRelationRuleBase *)>;

    mitkClassMacro(NodePredicateRuleFunction, NodePredicateBase)
      mitkNewMacro2Param(NodePredicateRuleFunction, const FunctionType &, PropertyRelationRuleBase::ConstPointer)

    ~NodePredicateRuleFunction() override = default;

    bool CheckNode(const mitk::DataNode *node) const override
    {
      if (!node)
      {
        return false;
      }

      return m_Function(node, m_Rule);
    };

  protected:
    explicit NodePredicateRuleFunction(const FunctionType &function, PropertyRelationRuleBase::ConstPointer rule) : m_Function(function), m_Rule(rule)
    {
    };

    FunctionType m_Function;
    PropertyRelationRuleBase::ConstPointer m_Rule;
  };

} // namespace mitk

mitk::NodePredicateBase::ConstPointer mitk::PropertyRelationRuleBase::GetSourceCandidateIndicator() const
{
  auto check = [](const mitk::IPropertyProvider *node, const mitk::PropertyRelationRuleBase *rule) {
    return rule->IsSourceCandidate(node);
  };

  return NodePredicateRuleFunction::New(check, this).GetPointer();
}

mitk::NodePredicateBase::ConstPointer mitk::PropertyRelationRuleBase::GetDestinationCandidateIndicator() const
{
  auto check = [](const mitk::IPropertyProvider *node, const mitk::PropertyRelationRuleBase *rule) {
    return rule->IsDestinationCandidate(node);
  };

  return NodePredicateRuleFunction::New(check, this).GetPointer();
}

mitk::NodePredicateBase::ConstPointer mitk::PropertyRelationRuleBase::GetConnectedSourcesDetector() const
{
  auto check = [](const mitk::IPropertyProvider *node, const mitk::PropertyRelationRuleBase *rule)
  {
    return rule->IsSource(node);
  };

  return NodePredicateRuleFunction::New(check, this).GetPointer();
}

mitk::NodePredicateBase::ConstPointer mitk::PropertyRelationRuleBase::GetSourcesDetector(
  const IPropertyProvider *destination, RelationType exclusiveRelation) const
{
  if (!destination)
  {
    mitkThrow() << "Error. Passed destination pointer is NULL";
  }

  auto check = [destination, exclusiveRelation](const mitk::IPropertyProvider *node,
                                              const mitk::PropertyRelationRuleBase *rule) {
    return rule->HasRelation(node, destination, exclusiveRelation);
  };

  return NodePredicateRuleFunction::New(check, this).GetPointer();
}

mitk::NodePredicateBase::ConstPointer mitk::PropertyRelationRuleBase::GetDestinationsDetector(
  const IPropertyProvider *source, RelationType exclusiveRelation) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }

  auto check = [source, exclusiveRelation](const mitk::IPropertyProvider *node,
                                         const mitk::PropertyRelationRuleBase *rule) {
    return rule->HasRelation(source, node, exclusiveRelation);
  };

  return NodePredicateRuleFunction::New(check, this).GetPointer();
}

mitk::NodePredicateBase::ConstPointer mitk::PropertyRelationRuleBase::GetDestinationDetector(
  const IPropertyProvider *source, RelationUIDType relationUID) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }

  auto relUIDs = this->GetExistingRelations(source);
  if (std::find(relUIDs.begin(), relUIDs.end(), relationUID) == relUIDs.end())
  {
    mitkThrow()
      << "Error. Passed relationUID does not identify a relation instance of the passed source for this rule instance.";
  };

  auto check = [source, relationUID](const mitk::IPropertyProvider *node, const mitk::PropertyRelationRuleBase *rule) {
    try
    {
      auto relevantUIDs = rule->GetRelationUIDs(source, node);
      for (const auto& aUID : relevantUIDs)
      {
        if (aUID == relationUID)
        {
          return true;
        }
      }
    }
    catch(const NoPropertyRelationException &)
    {
      return false;
    }
    return false;
  };

  return NodePredicateRuleFunction::New(check, this).GetPointer();
}

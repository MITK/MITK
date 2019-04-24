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

#include "mitkPropertyRelationRuleBase.h"

#include <mitkDataNode.h>
#include <mitkExceptionMacro.h>
#include <mitkNodePredicateBase.h>
#include <mitkStringProperty.h>
#include <mitkUIDGenerator.h>

#include <mutex>
#include <regex>

bool mitk::PropertyRelationRuleBase::IsAbstract() const
{
  return true;
};

bool mitk::PropertyRelationRuleBase::IsSourceCandidate(const IPropertyProvider *owner) const
{
  return owner != nullptr;
};

bool mitk::PropertyRelationRuleBase::IsDestinationCandidate(const IPropertyProvider *owner) const
{
  return owner != nullptr;
};

mitk::PropertyKeyPath mitk::PropertyRelationRuleBase::GetRootKeyPath()
{
  return PropertyKeyPath().AddElement("MITK").AddElement("Relations");
};

bool mitk::PropertyRelationRuleBase::IsSupportedRuleID(const RuleIDType& ruleID) const
{
  return ruleID == this->GetRuleID();
};


std::string mitk::PropertyRelationRuleBase::GetRIIPropertyRegEx(const std::string propName, const InstanceIDType &instanceID) const
{
  auto path = this->GetRootKeyPath();
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

  return PropertyKeyPathToPropertyRegEx(path);
};

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
};
//end workaround for T24729


bool mitk::PropertyRelationRuleBase::IsSource(const IPropertyProvider *owner) const
{
  if (!owner)
  {
    mitkThrow() << "Error. Passed owner pointer is NULL";
  }

  std::vector<std::string> keys;
  //workaround until T24729 is done. Please remove if T24728 is done
  keys = GetPropertyKeys(owner);
  //end workaround for T24729

  auto sourceRegExStr = this->GetRIIPropertyRegEx("ruleID");
  auto regEx = std::regex(sourceRegExStr);

  for (const auto &key : keys)
  {
    if (std::regex_match(key, regEx))
    {
      auto idProp = owner->GetConstProperty(key);
      auto ruleID = idProp->GetValueAsString();
      if (this->IsSupportedRuleID(ruleID))
      {
        return true;
      }
    }
  }
  
  return false;
};

mitk::PropertyRelationRuleBase::RelationType mitk::PropertyRelationRuleBase::HasRelation(
  const IPropertyProvider *source, const IPropertyProvider *destination) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }
  if (!destination)
  {
    mitkThrow() << "Error. Passed owner pointer is NULL";
  }

  RelationType result = RelationType::None;

  if (!this->GetInstanceID_IDLayer(source, destination).empty())
  { // has relations of type Connected_ID;
    result = RelationType::Connected_ID;
  }

  if (result == RelationType::None)
  {
    auto instanceIDs_data = this->GetInstanceID_datalayer(source, destination);

    if (instanceIDs_data.size() > 1)
    {
      MITK_WARN << "Property relation on data level is ambiguous. First relation is used. Instance ID: "
                << instanceIDs_data.front();
    }

    if (!instanceIDs_data.empty() && instanceIDs_data.front() != NULL_INSTANCE_ID())
    { // has relations of type Connected_Data;
      result = RelationType::Connected_Data;
    }
  }

  if (result == RelationType::None)
  {
    if (this->HasImplicitDataRelation(source, destination))
    { // has relations of type Connected_Data;
      result = RelationType::Implicit_Data;
    }
  }

  return result;
};

mitk::PropertyRelationRuleBase::RelationUIDVectorType mitk::PropertyRelationRuleBase::GetExistingRelations(
  const IPropertyProvider *source) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }

  auto ruleIDRegExStr = this->GetRIIPropertyRegEx("ruleID");
  auto regEx = std::regex(ruleIDRegExStr);

  //workaround until T24729 is done. You can use directly source->GetPropertyKeys again, when fixed.
  const auto keys = GetPropertyKeys(source);
  //end workaround for T24729

  RelationUIDVectorType relationUIDs;

  for (const auto &key : keys)
  {
    if (std::regex_match(key, regEx))
    {
      auto idProp = source->GetConstProperty(key);
      auto ruleID = idProp->GetValueAsString();
      if (this->IsSupportedRuleID(ruleID))
      {
        auto instanceID = this->GetInstanceIDByPropertyName(key);
        relationUIDs.push_back(this->GetRelationUIDByInstanceID(source, instanceID));
      }
    }
  }

  return relationUIDs;
};

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

  RelationUIDVectorType result;

  auto instanceIDs = this->GetInstanceID_IDLayer(source, destination);
  for (const auto instanceID : instanceIDs)
  {
    result.push_back(this->GetRelationUIDByInstanceID(source, instanceID));
  }

  if (result.empty() || this->IsAbstract())
  {
    auto instanceIDs_data = this->GetInstanceID_datalayer(source, destination);

    for (const auto instanceID : instanceIDs_data)
    {
      if (std::find(std::begin(instanceIDs), std::end(instanceIDs), instanceID) == std::end(instanceIDs))
      {
        result.push_back(this->GetRelationUIDByInstanceID(source, instanceID));
      }
    }
  }

  return result;
};

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
};

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
      PropertyKeyPathToPropertyName(this->GetRootKeyPath().AddElement(instanceID).AddElement("relationUID")));

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
};

mitk::PropertyRelationRuleBase::InstanceIDType mitk::PropertyRelationRuleBase::GetInstanceIDByRelationUID(
  const IPropertyProvider *source, const RelationUIDType &relationUID) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }

  InstanceIDType result = NULL_INSTANCE_ID();

  auto destRegExStr =
    PropertyKeyPathToPropertyRegEx(GetRootKeyPath().AddAnyElement().AddElement("relationUID"));
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
};

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

  auto identifiable = dynamic_cast<const Identifiable *>(destination);

  if (!identifiable)
  { //This check and pass through to data is needed due to solve T25711. See Task for more information.
    //This could be removed at the point we can get rid of DataNodes or they get realy transparent.
    auto node = dynamic_cast<const DataNode*>(destination);
    if (node && node->GetData())
    {
      identifiable = dynamic_cast<const Identifiable *>(node->GetData());
    }
  }

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
};

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

  auto instanceIDs_data = this->GetInstanceID_datalayer(source, destination);
  if (instanceIDs_data.size() > 1)
  {
    MITK_WARN << "Property relation on data level is ambiguous. First relation is used. Instance ID: "
              << instanceIDs_data.front();
  }
  bool hasDatalayer = !instanceIDs_data.empty();

  if (hasIDlayer && hasDatalayer && instanceIDs.front() != instanceIDs_data.front())
  {
    mitkThrow() << "Property relation information is in an invalid state. ID and data layer point to different "
                   "relation instances. Rule: "
                << this->GetRuleID() << "; ID based instance: " << instanceIDs.front()
                << "; Data base instance: " << instanceIDs_data.front();
  }

  RelationUIDType relationUID = this->CreateRelationUID();

  InstanceIDType instanceID = "";

  if (hasIDlayer)
  {
    instanceID = instanceIDs.front();
  }
  else if (hasDatalayer)
  {
    instanceID = instanceIDs_data.front();
  }
  else
  {
    instanceID = this->CreateNewRelationInstance(source, relationUID);
  }

  auto relUIDKey =
    PropertyKeyPathToPropertyName(GetRootKeyPath().AddElement(instanceID).AddElement("relationUID"));
  source->SetProperty(relUIDKey, mitk::StringProperty::New(relationUID));

  auto ruleIDKey =
    PropertyKeyPathToPropertyName(GetRootKeyPath().AddElement(instanceID).AddElement("ruleID"));
  source->SetProperty(ruleIDKey, mitk::StringProperty::New(this->GetRuleID()));

  if (!hasIDlayer)
  {
    auto identifiable = dynamic_cast<const Identifiable *>(destination);

    if (!identifiable)
    { //This check and pass through to data is needed due to solve T25711. See Task for more information.
      //This could be removed at the point we can get rid of DataNodes or they get realy transparent.
      auto node = dynamic_cast<const DataNode*>(destination);
      if (node && node->GetData())
      {
        identifiable = dynamic_cast<const Identifiable *>(node->GetData());
      }
    }

    if (identifiable)
    {
      auto destUIDKey =
        PropertyKeyPathToPropertyName(GetRootKeyPath().AddElement(instanceID).AddElement("destinationUID"));
      source->SetProperty(destUIDKey, mitk::StringProperty::New(identifiable->GetUID()));
    }
  }

  if (!hasDatalayer)
  {
    this->Connect_datalayer(source, destination, instanceID);
  }

  return relationUID;
};

void mitk::PropertyRelationRuleBase::Disconnect(IPropertyOwner *source, const IPropertyProvider *destination) const
{
  try
  {
    const auto relationUIDs = this->GetRelationUIDs(source, destination);
    for (const auto relUID: relationUIDs)
    {
      this->Disconnect(source, relUID);
    }
  }
  catch (const NoPropertyRelationException &)
  {
    // nothing to do and no real error in context of disconnect.
  }
};

void mitk::PropertyRelationRuleBase::Disconnect(IPropertyOwner *source, RelationUIDType relationUID) const
{
  auto instanceID = this->GetInstanceIDByRelationUID(source, relationUID);

  if (instanceID != NULL_INSTANCE_ID())
  {
    this->Disconnect_datalayer(source, instanceID);

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
};

mitk::PropertyRelationRuleBase::RelationUIDType mitk::PropertyRelationRuleBase::CreateRelationUID()
{
  UIDGenerator generator;
  return generator.GetUID();
};

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
    PropertyKeyPathToPropertyRegEx(this->GetRootKeyPath().AddAnyElement().AddElement("relationUID"));
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
    PropertyKeyPathToPropertyName(this->GetRootKeyPath().AddElement(newID).AddElement("relationUID"));
  source->SetProperty(relUIDKey, mitk::StringProperty::New(relationUID));

  return newID;
};

itk::LightObject::Pointer mitk::PropertyRelationRuleBase::InternalClone() const
{
  return Superclass::InternalClone();
};


mitk::PropertyRelationRuleBase::InstanceIDType mitk::PropertyRelationRuleBase::GetInstanceIDByPropertyName(const std::string propName)
{
  auto proppath = PropertyNameToPropertyKeyPath(propName);
  auto ref = GetRootKeyPath();

  if (proppath.GetSize() < 3 || !(proppath.GetFirstNode() == ref.GetFirstNode()) || !(proppath.GetNode(1) == ref.GetNode(1)))
  {
    mitkThrow() << "Property name is not for a RII property or containes no instance ID. Wrong name: " << propName;
  }

  return proppath.GetNode(2).name;
};

mitk::PropertyRelationRuleBase::RuleIDType mitk::PropertyRelationRuleBase::GetRuleIDByInstanceID(const IPropertyProvider *source,
  const InstanceIDType &instanceID) const
{
  if (!source)
  {
    mitkThrow() << "Error. Source is invalid. Cannot deduce rule ID";
  }

  auto path = GetRootKeyPath().AddElement(instanceID).AddElement("ruleID");
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
};

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
};

mitk::NodePredicateBase::ConstPointer mitk::PropertyRelationRuleBase::GetDestinationCandidateIndicator() const
{
  auto check = [](const mitk::IPropertyProvider *node, const mitk::PropertyRelationRuleBase *rule) {
    return rule->IsDestinationCandidate(node);
  };

  return NodePredicateRuleFunction::New(check, this).GetPointer();
};

mitk::NodePredicateBase::ConstPointer mitk::PropertyRelationRuleBase::GetConnectedSourcesDetector() const
{
  auto check = [](const mitk::IPropertyProvider *node, const mitk::PropertyRelationRuleBase *rule)
  {
    return rule->IsSource(node);
  };

  return NodePredicateRuleFunction::New(check, this).GetPointer();
};

mitk::NodePredicateBase::ConstPointer mitk::PropertyRelationRuleBase::GetSourcesDetector(
  const IPropertyProvider *destination, RelationType minimalRelation) const
{
  if (!destination)
  {
    mitkThrow() << "Error. Passed destination pointer is NULL";
  }

  auto check = [destination, minimalRelation](const mitk::IPropertyProvider *node,
                                              const mitk::PropertyRelationRuleBase *rule) {
    return rule->HasRelation(node, destination) >= minimalRelation;
  };

  return NodePredicateRuleFunction::New(check, this).GetPointer();
};

mitk::NodePredicateBase::ConstPointer mitk::PropertyRelationRuleBase::GetDestinationsDetector(
  const IPropertyProvider *source, RelationType minimalRelation) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }

  auto check = [source, minimalRelation](const mitk::IPropertyProvider *node,
                                         const mitk::PropertyRelationRuleBase *rule) {
    return rule->HasRelation(source, node) >= minimalRelation;
  };

  return NodePredicateRuleFunction::New(check, this).GetPointer();
};

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
};

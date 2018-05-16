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

mitk::PropertyKeyPath mitk::PropertyRelationRuleBase::GetRuleRootKeyPath() const
{
  return GetRootKeyPath().AddElement(this->GetRuleID());
};

bool mitk::PropertyRelationRuleBase::IsSource(const IPropertyProvider *owner) const
{
  if (!owner)
  {
    mitkThrow() << "Error. Passed owner pointer is NULL";
  }

  auto keys = owner->GetPropertyKeys();

  auto rootkey = PropertyKeyPathToPropertyName(this->GetRuleRootKeyPath());

  for (const auto &key : keys)
  {
    if (key.find(rootkey) == 0)
    {
      return true;
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

  if (this->GetInstanceID_IDLayer(source, destination) != NULL_INSTANCE_ID())
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

  auto relIDRegExStr =
    PropertyKeyPathToPropertyRegEx(this->GetRuleRootKeyPath().AddAnyElement().AddElement("relationUID"));
  auto regEx = std::regex(relIDRegExStr);

  const auto keys = source->GetPropertyKeys();
  RelationUIDVectorType relationUIDs;

  for (const auto &key : keys)
  {
    if (std::regex_match(key, regEx))
    {
      auto idProp = source->GetConstProperty(key);
      relationUIDs.push_back(idProp->GetValueAsString());
    }
  }

  return relationUIDs;
};

mitk::PropertyRelationRuleBase::RelationUIDType mitk::PropertyRelationRuleBase::GetRelationUID(
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

  RelationUIDType result;

  auto instanceID = this->GetInstanceID_IDLayer(source, destination);

  if (instanceID == NULL_INSTANCE_ID())
  {
    auto instanceID_data = this->GetInstanceID_datalayer(source, destination);

    if (!instanceID_data.empty())
    {
      instanceID = instanceID_data.front();
    }

    if (instanceID_data.size() > 1)
    {
      MITK_WARN << "Property relation on data level is ambigious. First relation is used. Instance ID: " << instanceID;
    }
  }

  return this->GetRelationUIDByInstanceID(source, instanceID);
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
      PropertyKeyPathToPropertyName(this->GetRuleRootKeyPath().AddElement(instanceID).AddElement("relationUID")));

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

  InstanceIDType result;

  auto destRegExStr =
    PropertyKeyPathToPropertyRegEx(this->GetRuleRootKeyPath().AddAnyElement().AddElement("relationUID"));
  auto regEx = std::regex(destRegExStr);
  std::smatch instance_matches;

  auto keys = source->GetPropertyKeys();

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

mitk::PropertyRelationRuleBase::InstanceIDType mitk::PropertyRelationRuleBase::GetInstanceID_IDLayer(
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

  std::string result;

  if (identifiable)
  { // check for relations of type Connected_ID;

    auto destRegExStr =
      PropertyKeyPathToPropertyRegEx(this->GetRuleRootKeyPath().AddAnyElement().AddElement("destinationUID"));
    auto regEx = std::regex(destRegExStr);
    std::smatch instance_matches;

    auto destUID = identifiable->GetUID();

    auto keys = source->GetPropertyKeys();

    for (const auto &key : keys)
    {
      if (std::regex_search(key, instance_matches, regEx))
      {
        auto idProp = source->GetConstProperty(key);
        if (idProp->GetValueAsString() == destUID)
        {
          if (instance_matches.size()>1)
          {
            result = instance_matches[1];
            break;
          }
        }
      }
    }
  }

  return result;
};

void mitk::PropertyRelationRuleBase::Connect(IPropertyOwner *source, const IPropertyProvider *destination) const
{
  if (!source)
  {
    mitkThrow() << "Error. Passed source pointer is NULL";
  }
  if (!destination)
  {
    mitkThrow() << "Error. Passed destination pointer is NULL";
  }

  InstanceIDType instanceID = this->GetInstanceID_IDLayer(source, destination);
  bool hasIDlayer = instanceID != NULL_INSTANCE_ID();

  auto instanceIDs_data = this->GetInstanceID_datalayer(source, destination);
  if (instanceIDs_data.size() > 1)
  {
    MITK_WARN << "Property relation on data level is ambiguous. First relation is used. Instance ID: "
              << instanceIDs_data.front();
  }
  bool hasDatalayer = !instanceIDs_data.empty();

  if (hasIDlayer && hasDatalayer && instanceID != instanceIDs_data.front())
  {
    mitkThrow() << "Property relation information is in an invalid state. ID and data layer point to different "
                   "relation instances. Rule: "
                << this->GetRuleID() << "; ID based instance: " << instanceID
                << "; Data base instance: " << instanceIDs_data.front();
  }

  RelationUIDType relationUID = this->CreateRelationUID();

  if (!hasIDlayer)
  {
    if (hasDatalayer)
    {
      instanceID = instanceIDs_data.front();
    }
    else
    {
      instanceID = this->CreateNewRelationInstance(source, relationUID);
    }
  }

  auto relUIDKey =
    PropertyKeyPathToPropertyName(this->GetRuleRootKeyPath().AddElement(instanceID).AddElement("relationUID"));
  source->SetProperty(relUIDKey, mitk::StringProperty::New(relationUID));

  if (!hasIDlayer)
  {
    auto identifiable = dynamic_cast<const Identifiable *>(destination);

    if (identifiable)
    {
      auto destUIDKey =
        PropertyKeyPathToPropertyName(this->GetRuleRootKeyPath().AddElement(instanceID).AddElement("destinationUID"));
      source->SetProperty(destUIDKey, mitk::StringProperty::New(identifiable->GetUID()));
    }
  }

  if (!hasDatalayer)
  {
    this->Connect_datalayer(source, destination, instanceID);
  }
};

void mitk::PropertyRelationRuleBase::Disconnect(IPropertyOwner *source, const IPropertyProvider *destination) const
{
  try
  {
    this->Disconnect(source, this->GetRelationUID(source, destination));
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

    auto instancePrefix = PropertyKeyPathToPropertyName(this->GetRuleRootKeyPath().AddElement(instanceID));

    auto keys = source->GetPropertyKeys();

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
    PropertyKeyPathToPropertyRegEx(this->GetRuleRootKeyPath().AddAnyElement().AddElement("destinationUID"));
  auto regEx = std::regex(destRegExStr);
  std::smatch instance_matches;

  auto keys = source->GetPropertyKeys();

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
    PropertyKeyPathToPropertyName(this->GetRuleRootKeyPath().AddElement(newID).AddElement("relationUID"));
  source->SetProperty(relUIDKey, mitk::StringProperty::New(relationUID));

  return newID;
};

itk::LightObject::Pointer mitk::PropertyRelationRuleBase::InternalClone() const
{
  return Superclass::InternalClone();
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
      return rule->GetRelationUID(source, node) == relationUID;
    }
    catch(const NoPropertyRelationException &)
    {
      return false;
    }
  };

  return NodePredicateRuleFunction::New(check, this).GetPointer();
};

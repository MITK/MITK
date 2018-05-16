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

#include "mitkDataNode.h"
#include "mitkStringProperty.h"

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <regex>

/** This class is used to test PropertyRelationRuleBase and get access to internals where needed to test them as well.
 */
namespace mitk
{
  class TestRule : public mitk::PropertyRelationRuleBase
  {
  public:
    mitkClassMacro(TestRule, mitk::PropertyRelationRuleBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    using RuleIDType = PropertyRelationRuleBase::RuleIDType;
    using RelationUIDType = PropertyRelationRuleBase::RelationUIDType;
    using RelationUIDVectorType = PropertyRelationRuleBase::RelationUIDVectorType;

    RuleIDType GetRuleID() const override { return "TestRule"; };

    std::string GetDisplayName() const override { return "TestDisplayName"; }
    std::string GetSourceRoleName() const override { return "source role"; }
    std::string GetDestinationRoleName() const override { return "destination role"; }

    using Superclass::GetRuleRootKeyPath;

  protected:
    TestRule() = default;
    ~TestRule() override = default;

    using InstanceIDType = PropertyRelationRuleBase::InstanceIDType;
    using InstanceIDVectorType = PropertyRelationRuleBase::InstanceIDVectorType;

    InstanceIDVectorType GetInstanceID_datalayer(const IPropertyProvider *source,
                                                         const IPropertyProvider *destination) const override
    {
      InstanceIDVectorType result;

      auto destProp = destination->GetConstProperty("name");

      if (destProp.IsNotNull())
      {
        auto destRegExStr =
          PropertyKeyPathToPropertyRegEx(this->GetRuleRootKeyPath().AddAnyElement().AddElement("dataHandle"));
        auto regEx = std::regex(destRegExStr);
        std::smatch instance_matches;

        auto keys = source->GetPropertyKeys();

        for (const auto &key : keys)
        {
          if (std::regex_search(key, instance_matches, regEx))
          {
            auto sourceProp = source->GetConstProperty(key);
            if (sourceProp->GetValueAsString() == destProp->GetValueAsString())
            {
              if (instance_matches.size()>1)
              {
                result.push_back(instance_matches[1]);
              }
            }
          }
        }
      }
      return result;
    };

    bool HasImplicitDataRelation(const IPropertyProvider *source,
                                         const IPropertyProvider *destination) const override
    {
      auto destProp = destination->GetConstProperty("name");
      auto sourceProp = source->GetConstProperty("referencedName");

      return destProp.IsNotNull() && sourceProp.IsNotNull() &&
             destProp->GetValueAsString() == sourceProp->GetValueAsString();
    };

    void Connect_datalayer(IPropertyOwner *source,
                                   const IPropertyProvider *destination,
                                   const InstanceIDType &instanceID) const override
    {
      auto destProp = destination->GetConstProperty("name");

      source->SetProperty("referencedName", StringProperty::New(destProp->GetValueAsString()));
      source->SetProperty(
        PropertyKeyPathToPropertyName(this->GetRuleRootKeyPath().AddElement(instanceID).AddElement("dataHandle")),
        StringProperty::New(destProp->GetValueAsString()));
    };

    void Disconnect_datalayer(IPropertyOwner *source, const InstanceIDType & /*instanceID*/) const override
    {
      source->RemoveProperty("referencedName");
    };
  };
} // namespace mitk

class mitkPropertyRelationRuleBaseTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyRelationRuleBaseTestSuite);

  MITK_TEST(IsSourceCandidate);
  MITK_TEST(IsDestinationCandidate);
  MITK_TEST(IsSource);
  MITK_TEST(HasRelation);
  MITK_TEST(GetExistingRelations);
  MITK_TEST(GetRelationUID);
  MITK_TEST(GetSourceCandidateIndicator);
  MITK_TEST(GetDestinationCandidateIndicator);
  MITK_TEST(GetConnectedSourcesDetector);
  MITK_TEST(GetSourcesDetector);
  MITK_TEST(GetDestinationsDetector);
  MITK_TEST(GetDestinationDetector);
  MITK_TEST(Connect);
  MITK_TEST(Disconnect);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::TestRule::Pointer rule;

  mitk::DataNode::Pointer unRelated;
  mitk::DataNode::Pointer source_implicit_1;
  mitk::DataNode::Pointer source_data_1;
  mitk::DataNode::Pointer source_idOnly_1;
  mitk::DataNode::Pointer source_1;

  mitk::DataNode::Pointer source_multi;

  mitk::DataNode::Pointer source_otherRule;

  mitk::DataNode::Pointer dest_1;
  mitk::DataNode::Pointer dest_2;

  bool hasRelationProperties(mitk::IPropertyProvider *provider, std::string instance = "") const
  {
    auto keyPath = rule->GetRuleRootKeyPath();
    if (!instance.empty())
    {
      keyPath.AddElement(instance);
    }

    auto prefix = mitk::PropertyKeyPathToPropertyName(keyPath);
    auto keys = provider->GetPropertyKeys();

    for (const auto &key : keys)
    {
      if (key.find(prefix) == 0)
      {
        return true;
      }
    }

    return false;
  }

public:
  void setUp() override
  {
    rule = mitk::TestRule::New();

    unRelated = mitk::DataNode::New();
    unRelated->SetName("unRelated");

    dest_1 = mitk::DataNode::New();
    dest_1->SetName("dest_1");

    dest_2 = mitk::DataNode::New();
    dest_2->SetName("dest_2");

    source_implicit_1 = mitk::DataNode::New();
    source_implicit_1->AddProperty("referencedName", mitk::StringProperty::New(dest_1->GetName()));

    source_idOnly_1 = mitk::DataNode::New();
    std::string name = "MITK.Relations." + rule->GetRuleID() + ".1.relationUID";
    source_idOnly_1->AddProperty(name.c_str(), mitk::StringProperty::New("uid1"));
    name = "MITK.Relations." + rule->GetRuleID() + ".1.destinationUID";
    source_idOnly_1->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1->GetUID()));

    source_data_1 = source_implicit_1->Clone();
    name = "MITK.Relations." + rule->GetRuleID() + ".1.relationUID";
    source_data_1->AddProperty(name.c_str(), mitk::StringProperty::New("uid2"));
    name = "MITK.Relations." + rule->GetRuleID() + ".1.dataHandle";
    source_data_1->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1->GetName()));

    source_1 = source_data_1->Clone();
    name = "MITK.Relations." + rule->GetRuleID() + ".1.relationUID";
    source_1->AddProperty(name.c_str(), mitk::StringProperty::New("uid3"), nullptr, true);
    name = "MITK.Relations." + rule->GetRuleID() + ".1.destinationUID";
    source_1->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1->GetUID()));

    source_multi = source_1->Clone();
    name = "MITK.Relations." + rule->GetRuleID() + ".1.relationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New("uid4"), nullptr, true);
    name = "MITK.Relations." + rule->GetRuleID() + ".1.destinationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1->GetUID()));
    name = "MITK.Relations." + rule->GetRuleID() + ".4.relationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New("uid5"));
    name = "MITK.Relations." + rule->GetRuleID() + ".4.destinationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New(dest_2->GetUID()));
    name = "MITK.Relations." + rule->GetRuleID() + ".2.relationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New("uid6"));
    name = "MITK.Relations." + rule->GetRuleID() + ".2.destinationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New("unknown"));

    source_otherRule = mitk::DataNode::New();
    name = "MITK.Relations.otherRuleID.1.relationUID";
    source_otherRule->AddProperty(name.c_str(), mitk::StringProperty::New("uid7"));
    name = "MITK.Relations.otherRuleID.1.destinationUID";
    source_otherRule->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1->GetUID()));
  }

  void tearDown() override {}

  void IsSourceCandidate()
  {
    CPPUNIT_ASSERT(rule->IsSourceCandidate(mitk::DataNode::New()));
    CPPUNIT_ASSERT(!rule->IsSourceCandidate(nullptr));
  }

  void IsDestinationCandidate()
  {
    CPPUNIT_ASSERT(rule->IsDestinationCandidate(mitk::DataNode::New()));
    CPPUNIT_ASSERT(!rule->IsDestinationCandidate(nullptr));
  }

  void IsSource()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE(
      "Violated precondition (nullptr) does not throw.", rule->IsSource(nullptr), itk::ExceptionObject);

    CPPUNIT_ASSERT(!rule->IsSource(unRelated));
    CPPUNIT_ASSERT(!rule->IsSource(source_implicit_1));
    CPPUNIT_ASSERT(rule->IsSource(source_data_1));
    CPPUNIT_ASSERT(rule->IsSource(source_idOnly_1));
    CPPUNIT_ASSERT(rule->IsSource(source_1));
    CPPUNIT_ASSERT(rule->IsSource(source_multi));

    CPPUNIT_ASSERT(!rule->IsSource(source_otherRule));
  }

  void HasRelation()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->HasRelation(nullptr, dest_1),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->HasRelation(source_1, nullptr),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT(rule->HasRelation(source_1, unRelated) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(rule->HasRelation(unRelated, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(rule->HasRelation(source_otherRule, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);

    CPPUNIT_ASSERT(rule->HasRelation(source_implicit_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Implicit_Data);
    CPPUNIT_ASSERT(rule->HasRelation(source_data_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(rule->HasRelation(source_idOnly_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(rule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(rule->HasRelation(source_multi, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    CPPUNIT_ASSERT(rule->HasRelation(source_1, dest_2) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(rule->HasRelation(source_multi, dest_2) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
  }

  void GetExistingRelations()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->GetExistingRelations(nullptr),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT(rule->GetExistingRelations(unRelated).empty());
    CPPUNIT_ASSERT(rule->GetExistingRelations(source_otherRule).empty());
    CPPUNIT_ASSERT(rule->GetExistingRelations(source_implicit_1).empty());

    auto uids = rule->GetExistingRelations(source_idOnly_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid1");

    uids = rule->GetExistingRelations(source_data_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid2");

    uids = rule->GetExistingRelations(source_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid3");

    uids = rule->GetExistingRelations(source_multi);
    CPPUNIT_ASSERT(uids.size() == 3);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid4") != uids.end());
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid5") != uids.end());
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid6") != uids.end());
  }

  void GetRelationUID()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->GetRelationUID(nullptr, dest_1),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->GetRelationUID(source_1, nullptr),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE(
      "Violated precondition (no relation of connected_data or conncected_ID) does not throw.",
      rule->GetRelationUID(source_1, unRelated),
      mitk::NoPropertyRelationException);

    CPPUNIT_ASSERT_THROW_MESSAGE(
      "Violated precondition (no relation of connected_data or conncected_ID) does not throw.",
      rule->GetRelationUID(unRelated, dest_1),
      mitk::NoPropertyRelationException);

    CPPUNIT_ASSERT_THROW_MESSAGE(
      "Violated precondition (no relation of connected_data or conncected_ID) does not throw.",
      rule->GetRelationUID(source_otherRule, dest_1),
      mitk::NoPropertyRelationException);

    CPPUNIT_ASSERT(rule->GetRelationUID(source_idOnly_1, dest_1) == "uid1");
    CPPUNIT_ASSERT(rule->GetRelationUID(source_data_1, dest_1) == "uid2");
    CPPUNIT_ASSERT(rule->GetRelationUID(source_1, dest_1) == "uid3");
    CPPUNIT_ASSERT(rule->GetRelationUID(source_multi, dest_1) == "uid4");
    CPPUNIT_ASSERT(rule->GetRelationUID(source_multi, dest_2) == "uid5");
  }

  void GetSourceCandidateIndicator()
  {
    auto predicate = rule->GetSourceCandidateIndicator();

    CPPUNIT_ASSERT(predicate->CheckNode(mitk::DataNode::New()));
    CPPUNIT_ASSERT(!predicate->CheckNode(nullptr));
  }

  void GetDestinationCandidateIndicator()
  {
    auto predicate = rule->GetDestinationCandidateIndicator();

    CPPUNIT_ASSERT(predicate->CheckNode(mitk::DataNode::New()));
    CPPUNIT_ASSERT(!predicate->CheckNode(nullptr));
  }

  void GetConnectedSourcesDetector()
  {
    auto predicate = rule->GetConnectedSourcesDetector();

    CPPUNIT_ASSERT(!predicate->CheckNode(nullptr));
    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_data_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_multi));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule));
  }

  void GetSourcesDetector()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->GetSourcesDetector(nullptr),
                                 itk::ExceptionObject);

    auto predicate = rule->GetSourcesDetector(dest_1);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule));

    CPPUNIT_ASSERT(predicate->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_data_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_multi));

    predicate = rule->GetSourcesDetector(dest_1, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_data_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_multi));

    predicate = rule->GetSourcesDetector(dest_1, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_data_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_multi));

    predicate = rule->GetSourcesDetector(dest_2, mitk::PropertyRelationRuleBase::RelationType::Implicit_Data);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_data_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_multi));
  }

  void GetDestinationsDetector()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->GetDestinationsDetector(nullptr),
                                 itk::ExceptionObject);

    auto predicate = rule->GetDestinationsDetector(source_otherRule);
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_1));

    predicate = rule->GetDestinationsDetector(source_implicit_1);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    predicate =
      rule->GetDestinationsDetector(source_implicit_1, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_1));

    predicate = rule->GetDestinationsDetector(source_data_1);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    predicate =
      rule->GetDestinationsDetector(source_data_1, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    predicate =
      rule->GetDestinationsDetector(source_data_1, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_1));

    predicate = rule->GetDestinationsDetector(source_idOnly_1);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    predicate =
      rule->GetDestinationsDetector(source_idOnly_1, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    predicate =
      rule->GetDestinationsDetector(source_idOnly_1, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));

    predicate = rule->GetDestinationsDetector(source_1);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_2));
    predicate =
      rule->GetDestinationsDetector(source_1, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_2));
    predicate =
      rule->GetDestinationsDetector(source_1, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_2));

    predicate = rule->GetDestinationsDetector(source_multi);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    CPPUNIT_ASSERT(predicate->CheckNode(dest_2));
    predicate =
      rule->GetDestinationsDetector(source_multi, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    CPPUNIT_ASSERT(predicate->CheckNode(dest_2));
    predicate =
      rule->GetDestinationsDetector(source_multi, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    CPPUNIT_ASSERT(predicate->CheckNode(dest_2));
  }

  void GetDestinationDetector()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->GetDestinationDetector(nullptr, "uid1"),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (relation uid is invalid) does not throw.",
                                 rule->GetDestinationDetector(source_1, "invalid uid"),
                                 itk::ExceptionObject);

    auto predicate = rule->GetDestinationDetector(source_1, "uid3");
    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_2));

    predicate = rule->GetDestinationDetector(source_multi, "uid5");
    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_1));
    CPPUNIT_ASSERT(predicate->CheckNode(dest_2));
  }

  void Connect()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->Connect(nullptr, dest_1),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->Connect(source_1, nullptr),
                                 itk::ExceptionObject);

    // check upgrade of an implicit connection
    CPPUNIT_ASSERT(rule->HasRelation(source_implicit_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Implicit_Data);
    rule->Connect(source_implicit_1, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_implicit_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    // check upgrade of an data connection
    CPPUNIT_ASSERT(rule->HasRelation(source_data_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    rule->Connect(source_data_1, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_data_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    std::string name = "MITK.Relations." + rule->GetRuleID() + ".1.destinationUID";
    auto prop = source_data_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE(
      "Destination uid was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was not stored.", prop->GetValueAsString() == dest_1->GetUID());

    // check actualization of an id only connection
    rule->Connect(source_idOnly_1, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_idOnly_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT_MESSAGE("Additional relation was defined instead of updating exting one.",
                           rule->GetExistingRelations(source_1).size() == 1);
    name = "MITK.Relations." + rule->GetRuleID() + ".1.dataHandle";
    prop = source_idOnly_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE(
      "Data layer information was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect data layer information was not stored.",
                           prop->GetValueAsString() == dest_1->GetName());
    prop = source_idOnly_1->GetProperty("referencedName");
    CPPUNIT_ASSERT_MESSAGE(
      "Data layer information was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect data layer information was not stored.",
                           prop->GetValueAsString() == dest_1->GetName());

    // check actualization of an existing connection
    rule->Connect(source_1, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT_MESSAGE("Additional relation was defined instead of updating exting one.",
                           rule->GetExistingRelations(source_1).size() == 1);

    // check new connection
    rule->Connect(source_multi, unRelated);
    CPPUNIT_ASSERT(rule->HasRelation(source_multi, unRelated) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    name = "MITK.Relations." + rule->GetRuleID() + ".5.dataHandle";
    prop = source_multi->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE(
      "Data layer information was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect data layer information was not stored.",
                           prop->GetValueAsString() == unRelated->GetName());
    prop = source_multi->GetProperty("referencedName");
    CPPUNIT_ASSERT_MESSAGE(
      "Data layer information was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect data layer information was not stored.",
                           prop->GetValueAsString() == unRelated->GetName());
    name = "MITK.Relations." + rule->GetRuleID() + ".5.destinationUID";
    prop = source_multi->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE(
      "Destination uid was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was not stored.", prop->GetValueAsString() == unRelated->GetUID());
  }

  void Disconnect()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->Disconnect(nullptr, dest_1),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->Disconnect(source_1, nullptr),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->Disconnect(nullptr, "uid"),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT(rule->HasRelation(source_1, unRelated) == mitk::PropertyRelationRuleBase::RelationType::None);
    rule->Disconnect(source_1, unRelated);
    CPPUNIT_ASSERT(rule->HasRelation(source_1, unRelated) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT_MESSAGE("Data property was not removed.", !source_1->GetProperty("referencedName"));

    rule->Disconnect(source_1, dest_2);
    CPPUNIT_ASSERT(rule->HasRelation(source_1, dest_2) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(this->hasRelationProperties(source_1));

    CPPUNIT_ASSERT(rule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    rule->Disconnect(source_1, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_1));

    CPPUNIT_ASSERT(rule->HasRelation(source_multi, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    rule->Disconnect(source_multi, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_multi, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_multi, "1"));
    CPPUNIT_ASSERT(this->hasRelationProperties(source_multi, "2"));
    CPPUNIT_ASSERT(this->hasRelationProperties(source_multi, "4"));

    rule->Disconnect(source_multi, "uid6");
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_multi, "1"));
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_multi, "2"));
    CPPUNIT_ASSERT(this->hasRelationProperties(source_multi, "4"));

    rule->Disconnect(source_multi, "unkownRelationUID");
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_multi, "1"));
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_multi, "2"));
    CPPUNIT_ASSERT(this->hasRelationProperties(source_multi, "4"));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyRelationRuleBase)

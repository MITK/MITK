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
#include "mitkPointSet.h"
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

    RuleIDType GetRuleID() const override
    {
      if (m_AbstractMode)
      {
        return "TestRule";
      }
      else
      {
        return "TestRule_type1";
      }
    };

    std::string GetDisplayName() const override { return "TestDisplayName"; }
    std::string GetSourceRoleName() const override { return "source role"; }
    std::string GetDestinationRoleName() const override { return "destination role"; }

    bool m_AbstractMode;

    bool IsAbstract() const override { return m_AbstractMode; }

    using Superclass::GetRootKeyPath;
    using Superclass::Connect;

  protected:
    TestRule() : m_AbstractMode(false)
    {
    };

    ~TestRule() override = default;

    using InstanceIDType = PropertyRelationRuleBase::InstanceIDType;
    using InstanceIDVectorType = PropertyRelationRuleBase::InstanceIDVectorType;

    bool IsSupportedRuleID(const RuleIDType& ruleID) const override
    {
      if (m_AbstractMode)
      {
        return ruleID.find(this->GetRuleID()) == 0;
      }
      else
      {
        return Superclass::IsSupportedRuleID(ruleID);
      }
    };

    InstanceIDVectorType GetInstanceID_datalayer(const IPropertyProvider *source,
                                                         const IPropertyProvider *destination) const override
    {
      InstanceIDVectorType result;

      auto destProp = destination->GetConstProperty("name");

      if (destProp.IsNotNull())
      {
        auto destRegExStr =
          PropertyKeyPathToPropertyRegEx(Superclass::GetRootKeyPath().AddAnyElement().AddElement("dataHandle"));
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
        PropertyKeyPathToPropertyName(Superclass::GetRootKeyPath().AddElement(instanceID).AddElement("dataHandle")),
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

  MITK_TEST(GetRootKeyPath);
  MITK_TEST(IsSourceCandidate);
  MITK_TEST(IsDestinationCandidate);
  MITK_TEST(IsSource);
  MITK_TEST(HasRelation);
  MITK_TEST(GetExistingRelations);
  MITK_TEST(GetRelationUIDs);
  MITK_TEST(GetSourceCandidateIndicator);
  MITK_TEST(GetDestinationCandidateIndicator);
  MITK_TEST(GetConnectedSourcesDetector);
  MITK_TEST(GetSourcesDetector);
  MITK_TEST(GetDestinationsDetector);
  MITK_TEST(GetDestinationDetector);
  MITK_TEST(Connect);
  MITK_TEST(Disconnect);
  MITK_TEST(Connect_abstract);
  MITK_TEST(Disconnect_abstract);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::TestRule::Pointer rule;
  mitk::TestRule::Pointer abstractRule;

  mitk::DataNode::Pointer unRelated;
  mitk::PointSet::Pointer unRelated_1_data;

  mitk::DataNode::Pointer source_implicit_1;
  mitk::DataNode::Pointer source_data_1;
  mitk::DataNode::Pointer source_idOnly_1;
  mitk::DataNode::Pointer source_1;

  mitk::DataNode::Pointer source_multi;

  mitk::DataNode::Pointer source_otherRule;

  mitk::DataNode::Pointer source_otherTypeRule; //relevant for abstract rule checks. Abstract rule should see it concrete rule not.

  mitk::DataNode::Pointer dest_1;
  mitk::PointSet::Pointer dest_1_data;
  mitk::DataNode::Pointer dest_2;
  mitk::PointSet::Pointer dest_2_data;

  bool hasRelationProperties(mitk::IPropertyProvider *provider, std::string instance = "") const
  {
    auto keyPath = mitk::PropertyRelationRuleBase::GetRootKeyPath();
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

    abstractRule = mitk::TestRule::New();
    abstractRule->m_AbstractMode = true;

    unRelated = mitk::DataNode::New();
    unRelated->SetName("unRelated");
    unRelated_1_data = mitk::PointSet::New();
    unRelated->SetData(unRelated_1_data);

    dest_1 = mitk::DataNode::New();
    dest_1->SetName("dest_1");
    dest_1_data = mitk::PointSet::New();
    dest_1->SetData(dest_1_data);

    dest_2 = mitk::DataNode::New();
    dest_2->SetName("dest_2");
    dest_2_data = mitk::PointSet::New();
    dest_2->SetData(dest_2_data);

    source_implicit_1 = mitk::DataNode::New();
    source_implicit_1->AddProperty("referencedName", mitk::StringProperty::New(dest_1->GetName()));

    source_idOnly_1 = mitk::DataNode::New();
    std::string name = "MITK.Relations.1.relationUID";
    source_idOnly_1->AddProperty(name.c_str(), mitk::StringProperty::New("uid1"));
    name = "MITK.Relations.1.destinationUID";
    source_idOnly_1->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1_data->GetUID()));
    name = "MITK.Relations.1.ruleID";
    source_idOnly_1->AddProperty(name.c_str(), mitk::StringProperty::New(rule->GetRuleID()));

    source_data_1 = source_implicit_1->Clone();
    name = "MITK.Relations.1.relationUID";
    source_data_1->AddProperty(name.c_str(), mitk::StringProperty::New("uid2"));
    name = "MITK.Relations.1.dataHandle";
    source_data_1->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1->GetName()));
    name = "MITK.Relations.1.ruleID";
    source_data_1->AddProperty(name.c_str(), mitk::StringProperty::New(rule->GetRuleID()));
    name = "MITK.Relations.2.relationUID";
    source_data_1->AddProperty(name.c_str(), mitk::StringProperty::New("uid10"), nullptr, true);
    name = "MITK.Relations.2.destinationUID";
    source_data_1->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1_data->GetUID()));
    name = "MITK.Relations.2.ruleID";
    source_data_1->AddProperty(name.c_str(), mitk::StringProperty::New("TestRule_othertype"));

    source_1 = source_data_1->Clone();
    name = "MITK.Relations.1.relationUID";
    source_1->AddProperty(name.c_str(), mitk::StringProperty::New("uid3"), nullptr, true);
    name = "MITK.Relations.1.destinationUID";
    source_1->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1_data->GetUID()));
    name = "MITK.Relations.1.ruleID";
    source_1->AddProperty(name.c_str(), mitk::StringProperty::New(rule->GetRuleID()));
    name = "MITK.Relations.2.relationUID";
    source_1->AddProperty(name.c_str(), mitk::StringProperty::New("uid8"), nullptr, true);
    name = "MITK.Relations.2.destinationUID";
    source_1->AddProperty(name.c_str(), mitk::StringProperty::New(dest_2_data->GetUID()));
    name = "MITK.Relations.2.ruleID";
    source_1->AddProperty(name.c_str(), mitk::StringProperty::New("TestRule_othertype"));

    source_multi = source_1->Clone();
    name = "MITK.Relations.1.relationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New("uid4"), nullptr, true);
    name = "MITK.Relations.1.destinationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1_data->GetUID()));
    name = "MITK.Relations.1.ruleID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New(rule->GetRuleID()));
    name = "MITK.Relations.4.relationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New("uid5"));
    name = "MITK.Relations.4.destinationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New(dest_2_data->GetUID()));
    name = "MITK.Relations.4.ruleID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New(rule->GetRuleID()));
    name = "MITK.Relations.2.relationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New("uid6"));
    name = "MITK.Relations.2.destinationUID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New("unknown"));
    name = "MITK.Relations.2.ruleID";
    source_multi->AddProperty(name.c_str(), mitk::StringProperty::New(rule->GetRuleID()));

    source_otherRule = mitk::DataNode::New();
    name = "MITK.Relations.1.relationUID";
    source_otherRule->AddProperty(name.c_str(), mitk::StringProperty::New("uid7"));
    name = "MITK.Relations.1.destinationUID";
    source_otherRule->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1_data->GetUID()));
    name = "MITK.Relations.1.ruleID";
    source_otherRule->AddProperty(name.c_str(), mitk::StringProperty::New("otherRuleID"));

    source_otherTypeRule = mitk::DataNode::New();
    name = "MITK.Relations.1.relationUID";
    source_otherTypeRule->AddProperty(name.c_str(), mitk::StringProperty::New("uid9"));
    name = "MITK.Relations.1.destinationUID";
    source_otherTypeRule->AddProperty(name.c_str(), mitk::StringProperty::New(dest_1_data->GetUID()));
    name = "MITK.Relations.1.ruleID";
    source_otherTypeRule->AddProperty(name.c_str(), mitk::StringProperty::New("TestRule_othertype"));
  }

  void tearDown() override {}

  void GetRootKeyPath()
  {
    mitk::PropertyKeyPath ref;
    ref.AddElement("MITK").AddElement("Relations");
    CPPUNIT_ASSERT(mitk::PropertyRelationRuleBase::GetRootKeyPath() == ref);
  }

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
    CPPUNIT_ASSERT(!rule->IsSource(source_otherTypeRule));

    CPPUNIT_ASSERT(!abstractRule->IsSource(unRelated));
    CPPUNIT_ASSERT(!abstractRule->IsSource(source_implicit_1));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_data_1));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_idOnly_1));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_1));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_multi));

    CPPUNIT_ASSERT(!abstractRule->IsSource(source_otherRule));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_otherTypeRule));
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
    CPPUNIT_ASSERT(rule->HasRelation(source_otherTypeRule, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);

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


    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, unRelated) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(abstractRule->HasRelation(unRelated, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_otherRule, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_otherTypeRule, dest_1) ==
      mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    CPPUNIT_ASSERT(abstractRule->HasRelation(source_implicit_1, dest_1) ==
      mitk::PropertyRelationRuleBase::RelationType::Implicit_Data);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_data_1, dest_1) ==
      mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_idOnly_1, dest_1) ==
      mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_multi, dest_1) ==
      mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_2) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_multi, dest_2) ==
      mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
  }

  void GetExistingRelations()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->GetExistingRelations(nullptr),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT(rule->GetExistingRelations(unRelated).empty());
    CPPUNIT_ASSERT(rule->GetExistingRelations(source_otherRule).empty());
    CPPUNIT_ASSERT(rule->GetExistingRelations(source_otherTypeRule).empty());
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


    CPPUNIT_ASSERT(abstractRule->GetExistingRelations(unRelated).empty());
    CPPUNIT_ASSERT(abstractRule->GetExistingRelations(source_otherRule).empty());
    CPPUNIT_ASSERT(abstractRule->GetExistingRelations(source_implicit_1).empty());

    uids = abstractRule->GetExistingRelations(source_idOnly_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid1");

    uids = abstractRule->GetExistingRelations(source_data_1);
    CPPUNIT_ASSERT(uids.size() == 2);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid2") != uids.end());
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid10") != uids.end());

    uids = abstractRule->GetExistingRelations(source_1);
    CPPUNIT_ASSERT(uids.size() == 2);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid3") != uids.end());
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid8") != uids.end());

    uids = abstractRule->GetExistingRelations(source_multi);
    CPPUNIT_ASSERT(uids.size() == 3);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid4") != uids.end());
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid5") != uids.end());
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid6") != uids.end());

    uids = abstractRule->GetExistingRelations(source_otherTypeRule);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid9");

  }

  void GetRelationUIDs()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->GetRelationUIDs(nullptr, dest_1),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->GetRelationUIDs(source_1, nullptr),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_1, unRelated).empty());
    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_1, dest_2).empty());
    CPPUNIT_ASSERT(rule->GetRelationUIDs(unRelated, dest_1).empty());
    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_otherRule, dest_1).empty());
    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_otherTypeRule, dest_1).empty());

    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_idOnly_1, dest_1).front() == "uid1");
    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_data_1, dest_1).front() == "uid2");
    auto uids = rule->GetRelationUIDs(source_1, dest_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid3");
    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_multi, dest_1).front() == "uid4");
    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_multi, dest_2).front() == "uid5");

    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(source_1, unRelated).empty());
    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(unRelated, dest_1).empty());
    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(source_otherRule, dest_1).empty());
    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(source_otherTypeRule, dest_1).front() == "uid9");

    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(source_idOnly_1, dest_1).front() == "uid1");
    uids = abstractRule->GetRelationUIDs(source_data_1, dest_1);
    CPPUNIT_ASSERT(uids.size() == 2);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid2") != uids.end());
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid10") != uids.end());
    uids = abstractRule->GetRelationUIDs(source_1, dest_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid3") != uids.end());
    uids = abstractRule->GetRelationUIDs(source_1, dest_2);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid8") != uids.end());
    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(source_1, dest_1).front() == "uid3");
    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(source_multi, dest_1).front() == "uid4");
    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(source_multi, dest_2).front() == "uid5");

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
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherTypeRule));


    auto predicate2 = abstractRule->GetConnectedSourcesDetector();

    CPPUNIT_ASSERT(!predicate2->CheckNode(nullptr));
    CPPUNIT_ASSERT(!predicate2->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate2->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(predicate2->CheckNode(source_data_1));
    CPPUNIT_ASSERT(predicate2->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(predicate2->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate2->CheckNode(source_multi));

    CPPUNIT_ASSERT(!predicate2->CheckNode(source_otherRule));
    CPPUNIT_ASSERT(predicate2->CheckNode(source_otherTypeRule));
  }

  void GetSourcesDetector()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->GetSourcesDetector(nullptr),
                                 itk::ExceptionObject);

    auto predicate = rule->GetSourcesDetector(dest_1);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherTypeRule));

    CPPUNIT_ASSERT(predicate->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_data_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_multi));

    predicate = rule->GetSourcesDetector(dest_1, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherTypeRule));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_data_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_multi));

    predicate = rule->GetSourcesDetector(dest_1, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherTypeRule));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_data_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_multi));

    predicate = rule->GetSourcesDetector(dest_2, mitk::PropertyRelationRuleBase::RelationType::Implicit_Data);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherTypeRule));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_data_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_multi));

    predicate = abstractRule->GetSourcesDetector(dest_1, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule));
    CPPUNIT_ASSERT(predicate->CheckNode(source_otherTypeRule));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_data_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_multi));

    predicate = abstractRule->GetSourcesDetector(dest_1);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule));
    CPPUNIT_ASSERT(predicate->CheckNode(source_otherTypeRule));

    CPPUNIT_ASSERT(predicate->CheckNode(source_implicit_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_data_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1));
    CPPUNIT_ASSERT(predicate->CheckNode(source_multi));
  }

  void GetDestinationsDetector()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->GetDestinationsDetector(nullptr),
                                 itk::ExceptionObject);

    auto predicate = rule->GetDestinationsDetector(source_otherRule);
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_1));

    predicate = rule->GetDestinationsDetector(source_otherTypeRule);
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

    predicate = abstractRule->GetDestinationsDetector(source_otherTypeRule);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));
    predicate = abstractRule->GetDestinationsDetector(source_otherTypeRule);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1));

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
    std::string name = "MITK.Relations.1.destinationUID";
    auto prop = source_data_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE(
      "Destination uid was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was  stored.", prop->GetValueAsString() == dest_1_data->GetUID());

    // check actualization of an id only connection
    rule->Connect(source_idOnly_1, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_idOnly_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT_MESSAGE("Additional relation was defined instead of updating exting one.",
                           rule->GetExistingRelations(source_1).size() == 1);
    name = "MITK.Relations.1.dataHandle";
    prop = source_idOnly_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE(
      "Data layer information was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect data layer information not stored.",
                           prop->GetValueAsString() == dest_1->GetName());
    prop = source_idOnly_1->GetProperty("referencedName");
    CPPUNIT_ASSERT_MESSAGE(
      "Data layer information was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect data layer information was stored.",
                           prop->GetValueAsString() == dest_1->GetName());

    // check actualization of an existing connection
    rule->Connect(source_1, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT_MESSAGE("Additional relation was defined instead of updating exting one.",
                           rule->GetExistingRelations(source_1).size() == 1);

    // check new connection
    auto newConnectUID = rule->Connect(source_multi, unRelated);
    CPPUNIT_ASSERT(rule->HasRelation(source_multi, unRelated) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    name = "MITK.Relations.5.dataHandle";
    prop = source_multi->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE(
      "Data layer information was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect data layer information was stored.",
                           prop->GetValueAsString() == unRelated->GetName());
    prop = source_multi->GetProperty("referencedName");
    CPPUNIT_ASSERT_MESSAGE(
      "Data layer information was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect data layer information was stored.",
                           prop->GetValueAsString() == unRelated->GetName());
    name = "MITK.Relations.5.destinationUID";
    prop = source_multi->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE(
      "Destination uid was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == unRelated_1_data->GetUID());

    auto storedRelationUIDs = rule->GetRelationUIDs(source_multi, unRelated);
    CPPUNIT_ASSERT_MESSAGE(
      "Relation uid was not stored for given source and destination.", storedRelationUIDs.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Incorrect Relation uid was stored.", storedRelationUIDs[0] == newConnectUID);
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
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_1, "1"));
    CPPUNIT_ASSERT_MESSAGE("Data of other rule type was removed.",this->hasRelationProperties(source_1, "2"));

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

    rule->Disconnect(source_otherTypeRule, dest_1);
    CPPUNIT_ASSERT_MESSAGE("Data of other rule type was removed.", this->hasRelationProperties(source_otherTypeRule, "1"));
  }

  void Connect_abstract()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (abstract does not connect) does not throw.",
      abstractRule->Connect(nullptr, dest_1),
      itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (abstract does not connect) does not throw.",
      abstractRule->Connect(source_1, nullptr),
      itk::ExceptionObject);
  }

  void Disconnect_abstract()
  {

    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_2) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    abstractRule->Disconnect(source_1, dest_2);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_2) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_1, "2"));

    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    abstractRule->Disconnect(source_1, dest_1);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_1, "1"));

    CPPUNIT_ASSERT(abstractRule->HasRelation(source_multi, dest_1) ==
      mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    abstractRule->Disconnect(source_multi, dest_1);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_multi, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_multi, "1"));
    CPPUNIT_ASSERT(this->hasRelationProperties(source_multi, "2"));
    CPPUNIT_ASSERT(this->hasRelationProperties(source_multi, "4"));

    abstractRule->Disconnect(source_multi, "uid6");
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_multi, "1"));
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_multi, "2"));
    CPPUNIT_ASSERT(this->hasRelationProperties(source_multi, "4"));

    abstractRule->Disconnect(source_multi, "unkownRelationUID");
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_multi, "1"));
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_multi, "2"));
    CPPUNIT_ASSERT(this->hasRelationProperties(source_multi, "4"));

    abstractRule->Disconnect(source_otherTypeRule, dest_1);
    CPPUNIT_ASSERT_MESSAGE("Data of other rule type was removed.", !this->hasRelationProperties(source_otherTypeRule, "1"));

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyRelationRuleBase)

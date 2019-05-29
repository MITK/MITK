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


#include "mitkPropertyRelations.h"
#include "mitkGenericIDRelationRule.h"
#include "mitkDataNode.h"

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <mitkEqual.h>
#include <mitkNumericConstants.h>

class mitkPropertyRelationsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyRelationsTestSuite);

  MITK_TEST(AddRule);
  MITK_TEST(GetRulesForSource);
  MITK_TEST(GetRulesForSourceCandidate);
  MITK_TEST(GetRulesForDestinationCandidate);
  MITK_TEST(GetRule);
  MITK_TEST(GetRuleIDs);
  MITK_TEST(HasRuleForSource);
  MITK_TEST(RemoveRule);
  MITK_TEST(RemoveAllRules);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::GenericIDRelationRule::Pointer rule1;
  mitk::GenericIDRelationRule::Pointer rule2;
  mitk::GenericIDRelationRule::Pointer rule3;
  mitk::GenericIDRelationRule::Pointer rule4;
  mitk::GenericIDRelationRule::Pointer unkownRule;

  mitk::DataNode::Pointer unRelated;
  mitk::DataNode::Pointer destination;
  mitk::DataNode::Pointer source_1;
  mitk::DataNode::Pointer source_1n2;
  mitk::DataNode::Pointer source_2;
  mitk::DataNode::Pointer source_4;

  mitk::IPropertyRelations *service;

  static bool checkExistence(const mitk::PropertyRelations::RuleResultVectorType &rules,
                             const mitk::PropertyRelationRuleBase *rule)
  {
    auto predicate = [rule](const mitk::PropertyRelationRuleBase::ConstPointer &x) { return rule->GetRuleID() == x->GetRuleID(); };

    auto finding = std::find_if(rules.begin(), rules.end(), predicate);

    bool result = finding != rules.end();
    return result;
  }

public:
  void setUp() override
  {
    service = mitk::CreateTestInstancePropertyRelations();

    rule1 = mitk::GenericIDRelationRule::New("rule1");
    rule2 = mitk::GenericIDRelationRule::New("rule2");
    rule3 = mitk::GenericIDRelationRule::New("rule3");
    rule4 = mitk::GenericIDRelationRule::New("rule4");
    unkownRule = mitk::GenericIDRelationRule::New("unkownRule");

    unRelated = mitk::DataNode::New();
    unRelated->SetName("unRelated");

    destination = mitk::DataNode::New();
    destination->SetName("destination");

    source_1 = mitk::DataNode::New();
    source_1->SetName("source_1");
    rule1->Connect(source_1, destination);

    source_1n2 = mitk::DataNode::New();
    source_1n2->SetName("source_1n2");
    rule1->Connect(source_1n2, destination);
    rule2->Connect(source_1n2, destination);

    source_2 = mitk::DataNode::New();
    source_2->SetName("source_2");
    rule2->Connect(source_2, destination);

    source_4 = mitk::DataNode::New();
    source_4->SetName("source_4");
    rule4->Connect(source_4, destination);

    service->AddRule(rule1, false);
    service->AddRule(rule2, false);
    service->AddRule(rule3, false);
    service->AddRule(rule4, false);
  }

  void tearDown() override { delete service; }

  void AddRule()
  {
    mitk::PropertyRelationRuleBase::Pointer rule2_new = mitk::GenericIDRelationRule::New("rule2").GetPointer();

    CPPUNIT_ASSERT_MESSAGE("Testing addrule of already existing rule (no overwrite) -> failed",
                           !service->AddRule(rule2_new, false));
    CPPUNIT_ASSERT(service->GetRule(rule2->GetRuleID()).GetPointer() == rule2.GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Testing addrule of already existing rule (overwrite) -> failed, was not added.",
                           service->AddRule(rule2_new, true));
    CPPUNIT_ASSERT(service->GetRule(rule2->GetRuleID()).GetPointer() == rule2_new.GetPointer());


    CPPUNIT_ASSERT(service->GetRule(unkownRule->GetRuleID()).IsNull());
    CPPUNIT_ASSERT(service->AddRule(unkownRule));
    CPPUNIT_ASSERT(service->GetRule(unkownRule->GetRuleID()).IsNotNull());

    CPPUNIT_ASSERT(!service->AddRule(nullptr));
  }

  void GetRulesForSource()
  {
    mitk::PropertyRelations::RuleResultVectorType rules = service->GetRulesForSource(source_1);
    CPPUNIT_ASSERT(rules.size() == 1);
    CPPUNIT_ASSERT(checkExistence(rules, rule1));

    rules = service->GetRulesForSource(source_1n2);
    CPPUNIT_ASSERT(rules.size() == 2);
    CPPUNIT_ASSERT(checkExistence(rules, rule1));
    CPPUNIT_ASSERT(checkExistence(rules, rule2));

    rules = service->GetRulesForSource(unRelated);
    CPPUNIT_ASSERT(rules.size() == 0);

    rules = service->GetRulesForSource(nullptr);
    CPPUNIT_ASSERT(rules.size() == 0);
  }

  void GetRulesForSourceCandidate()
  {
    mitk::PropertyRelations::RuleResultVectorType rules = service->GetRulesForSourceCandidate(unRelated);
    CPPUNIT_ASSERT(rules.size() == 4);
    CPPUNIT_ASSERT(checkExistence(rules, rule1));
    CPPUNIT_ASSERT(checkExistence(rules, rule2));
    CPPUNIT_ASSERT(checkExistence(rules, rule3));
    CPPUNIT_ASSERT(checkExistence(rules, rule4));

    rules = service->GetRulesForSourceCandidate(nullptr);
    CPPUNIT_ASSERT(rules.size() == 0);
  }

  void GetRulesForDestinationCandidate()
  {
    mitk::PropertyRelations::RuleResultVectorType rules = service->GetRulesForDestinationCandidate(unRelated);
    CPPUNIT_ASSERT(rules.size() == 4);
    CPPUNIT_ASSERT(checkExistence(rules, rule1));
    CPPUNIT_ASSERT(checkExistence(rules, rule2));
    CPPUNIT_ASSERT(checkExistence(rules, rule3));
    CPPUNIT_ASSERT(checkExistence(rules, rule4));

    rules = service->GetRulesForDestinationCandidate(nullptr);
    CPPUNIT_ASSERT(rules.size() == 0);
  }

  void GetRule()
  {
    auto rule = service->GetRule(rule3->GetRuleID());
    CPPUNIT_ASSERT(rule.IsNotNull());
    rule = service->GetRule(rule1->GetRuleID());
    CPPUNIT_ASSERT(rule.IsNotNull());
    rule = service->GetRule(rule2->GetRuleID());
    CPPUNIT_ASSERT(rule.IsNotNull());
    rule = service->GetRule(rule4->GetRuleID());
    CPPUNIT_ASSERT(rule.IsNotNull());

    rule = service->GetRule(unkownRule->GetRuleID());
    CPPUNIT_ASSERT(rule.IsNull());
  }

  void GetRuleIDs()
  {
    auto ruleIDs = service->GetRuleIDs();

    CPPUNIT_ASSERT(ruleIDs.size()==4);
    CPPUNIT_ASSERT(std::find(ruleIDs.begin(), ruleIDs.end(), rule1->GetRuleID()) != ruleIDs.end());
    CPPUNIT_ASSERT(std::find(ruleIDs.begin(), ruleIDs.end(), rule2->GetRuleID()) != ruleIDs.end());
    CPPUNIT_ASSERT(std::find(ruleIDs.begin(), ruleIDs.end(), rule3->GetRuleID()) != ruleIDs.end());
    CPPUNIT_ASSERT(std::find(ruleIDs.begin(), ruleIDs.end(), rule4->GetRuleID()) != ruleIDs.end());
  }

  void HasRuleForSource()
  {
    CPPUNIT_ASSERT(service->HasRuleForSource(source_1));
    CPPUNIT_ASSERT(service->HasRuleForSource(source_1n2));
    CPPUNIT_ASSERT(!service->HasRuleForSource(unRelated));
    CPPUNIT_ASSERT(!service->HasRuleForSource(nullptr));
  }

  void RemoveRule()
  {
    CPPUNIT_ASSERT_NO_THROW(service->RemoveRule(rule1->GetRuleID()));
    CPPUNIT_ASSERT(!service->GetRule(rule1->GetRuleID()));
    CPPUNIT_ASSERT(service->GetRule(rule2->GetRuleID()));
    CPPUNIT_ASSERT(service->GetRule(rule3->GetRuleID()));
    CPPUNIT_ASSERT(service->GetRule(rule4->GetRuleID()));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveRule(rule3->GetRuleID()));
    CPPUNIT_ASSERT(!service->GetRule(rule1->GetRuleID()));
    CPPUNIT_ASSERT(service->GetRule(rule2->GetRuleID()));
    CPPUNIT_ASSERT(!service->GetRule(rule3->GetRuleID()));
    CPPUNIT_ASSERT(service->GetRule(rule4->GetRuleID()));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveRule("unknown_rule"));
    CPPUNIT_ASSERT(!service->GetRule(rule1->GetRuleID()));
    CPPUNIT_ASSERT(service->GetRule(rule2->GetRuleID()));
    CPPUNIT_ASSERT(!service->GetRule(rule3->GetRuleID()));
    CPPUNIT_ASSERT(service->GetRule(rule4->GetRuleID()));
  }

  void RemoveAllRules()
  {
    CPPUNIT_ASSERT_NO_THROW(service->RemoveAllRules());
    CPPUNIT_ASSERT(service->GetRuleIDs().empty());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyRelations)

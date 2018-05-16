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

#include "mitkGenericIDRelationRule.h"

#include "mitkDataNode.h"
#include "mitkStringProperty.h"

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

class mitkGenericIDRelationRuleTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGenericIDRelationRuleTestSuite);

  MITK_TEST(GetRuleID);
  MITK_TEST(GetDisplayName);
  MITK_TEST(GetSourceRoleName);
  MITK_TEST(GetDestinationRoleName);
  MITK_TEST(Clone);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::GenericIDRelationRule::Pointer rule_new1;
  mitk::GenericIDRelationRule::Pointer rule_new2;
  mitk::GenericIDRelationRule::Pointer rule_new4;

public:
  void setUp() override
  {
    rule_new1 = mitk::GenericIDRelationRule::New("TestID");
    rule_new2 = mitk::GenericIDRelationRule::New("TestID2", "display name");
    rule_new4 = mitk::GenericIDRelationRule::New("TestID4", "display name 4", "srole", "drole");
  }

  void tearDown() override {}

  void GetRuleID()
  {
    CPPUNIT_ASSERT(rule_new1->GetRuleID() == "IDRelation_TestID");
    CPPUNIT_ASSERT(rule_new2->GetRuleID() == "IDRelation_TestID2");
    CPPUNIT_ASSERT(rule_new4->GetRuleID() == "IDRelation_TestID4");
  }

  void GetDisplayName()
  {
    CPPUNIT_ASSERT(rule_new1->GetDisplayName() == "TestID relation");
    CPPUNIT_ASSERT(rule_new2->GetDisplayName() == "display name");
    CPPUNIT_ASSERT(rule_new4->GetDisplayName() == "display name 4");
  }

  void GetSourceRoleName()
  {
    CPPUNIT_ASSERT(rule_new1->GetSourceRoleName() == "source of TestID relation");
    CPPUNIT_ASSERT(rule_new2->GetSourceRoleName() == "source of TestID2 relation");
    CPPUNIT_ASSERT(rule_new4->GetSourceRoleName() == "srole");
  }

  void GetDestinationRoleName()
  {
    CPPUNIT_ASSERT(rule_new1->GetDestinationRoleName() == "destination of TestID relation");
    CPPUNIT_ASSERT(rule_new2->GetDestinationRoleName() == "destination of TestID2 relation");
    CPPUNIT_ASSERT(rule_new4->GetDestinationRoleName() == "drole");
  }

  void Clone()
  {
    auto clone = rule_new4->Clone();

    CPPUNIT_ASSERT(clone->GetRuleID() == "IDRelation_TestID4");
    CPPUNIT_ASSERT(clone->GetDisplayName() == "display name 4");
    CPPUNIT_ASSERT(clone->GetSourceRoleName() == "srole");
    CPPUNIT_ASSERT(clone->GetDestinationRoleName() == "drole");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGenericIDRelationRule)

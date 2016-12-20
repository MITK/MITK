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

#include "mitkGeometry3D.h"
#include "mitkBaseDataTestImplementation.h"
#include "mitkNodePredicateDataProperty.h"
#include "mitkDataNode.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
#include "mitkStringProperty.h"


class mitkNodePredicateDataPropertyTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNodePredicateDataPropertyTestSuite);
  MITK_TEST(Check_InvalidConstructor);
  MITK_TEST(Check_InvalidCheckNode);
  MITK_TEST(Check_CheckName);
  MITK_TEST(Check_CheckNameAndValue);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::BaseDataTestImplementation::Pointer m_Data;
  mitk::BaseDataTestImplementation::Pointer m_OtherData;
  mitk::DataNode::Pointer m_Node;
  mitk::DataNode::Pointer m_NullNode;
  mitk::DataNode::Pointer m_NullDataNode;
  mitk::DataNode::Pointer m_OtherNode;
  mitk::DataNode::Pointer m_NoPropNode;

  mitk::StringProperty::Pointer m_RefProperty;

public:
  /**
  * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members
  * for a new test case. (If the members are not used in a test, the method does not need to be called).
  */
  void setUp() override
  {
    m_RefProperty = mitk::StringProperty::New("value1");

    m_Data = mitk::BaseDataTestImplementation::New();
    m_Data->SetProperty("prop1", mitk::StringProperty::New("value1"));
    m_Data->SetProperty("prop2", mitk::StringProperty::New("value2"));

    m_OtherData = mitk::BaseDataTestImplementation::New();
    m_OtherData->SetProperty("prop1", mitk::StringProperty::New("othervalue"));

    m_NullNode = nullptr;

    m_NullDataNode = mitk::DataNode::New();

    m_Node = mitk::DataNode::New();
    m_Node->SetData(m_Data);

    m_OtherNode = mitk::DataNode::New();
    m_OtherNode->SetData(m_OtherData);

    m_NoPropNode = mitk::DataNode::New();
    m_NoPropNode->SetData(mitk::BaseDataTestImplementation::New());
  }

  void tearDown() override
  {
    m_Node = nullptr;
    m_NullNode = nullptr;
    m_NullDataNode = nullptr;
    m_OtherNode = nullptr;
    m_Data = nullptr;
    m_OtherData = nullptr;
  }

  void Check_InvalidConstructor()
  {
    CPPUNIT_ASSERT_THROW(mitk::NodePredicateDataProperty::New(""), mitk::Exception);
  }

  void Check_InvalidCheckNode()
  {
    mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New("aProp");

    CPPUNIT_ASSERT_THROW(predicate->CheckNode(nullptr), mitk::Exception);
  }

  void Check_CheckName()
  {
    mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New("prop1");

    CPPUNIT_ASSERT(!predicate->CheckNode(m_NullDataNode));
    CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(m_OtherNode));
    CPPUNIT_ASSERT(!predicate->CheckNode(m_NoPropNode));
  }

  void Check_CheckNameAndValue()
  {
    mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New("prop1", m_RefProperty);

    CPPUNIT_ASSERT(!predicate->CheckNode(m_NullDataNode));
    CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(m_OtherNode));
    CPPUNIT_ASSERT(!predicate->CheckNode(m_NoPropNode));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkNodePredicateDataProperty)

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

#include <mitkDataNode.h>
#include <mitkNodePredicateFunction.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

bool FreeFunction(const mitk::DataNode *node)
{
  return nullptr != node;
}

struct FunctionObject
{
  bool operator()(const mitk::DataNode *node) const
  {
    return nullptr != node;
  }
};

class mitkNodePredicateFunctionTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNodePredicateFunctionTestSuite);
  MITK_TEST(CheckFreeFunction);
  MITK_TEST(CheckFunctionObject);
  MITK_TEST(CheckMemberFunction);
  MITK_TEST(CheckLambdaExpression);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataNode::Pointer m_NullNode;
  mitk::DataNode::Pointer m_Node;

  bool MemberFunction(const mitk::DataNode *node) const
  {
    return nullptr != node;
  }

  void Check(mitk::NodePredicateBase *predicate) const
  {
    CPPUNIT_ASSERT(!predicate->CheckNode(m_NullNode));
    CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
  }

public:
  void setUp() override
  {
    m_NullNode = nullptr;
    m_Node = mitk::DataNode::New();
  }

  void tearDown() override
  {
    m_NullNode = nullptr;
    m_Node = nullptr;
  }

  void CheckFreeFunction()
  {
    auto predicate = mitk::NodePredicateFunction::New(FreeFunction);
    this->Check(predicate);
  }

  void CheckFunctionObject()
  {
    FunctionObject functionObject;
    auto predicate = mitk::NodePredicateFunction::New(functionObject);
    this->Check(predicate);
  }

  void CheckMemberFunction()
  {
    using namespace std::placeholders;
    auto memberFunction = std::bind(&mitkNodePredicateFunctionTestSuite::MemberFunction, this, _1);
    auto predicate = mitk::NodePredicateFunction::New(memberFunction);
    this->Check(predicate);
  }

  void CheckLambdaExpression()
  {
    auto predicate = mitk::NodePredicateFunction::New([](const mitk::DataNode *node) {
      return nullptr != node;
    });

    this->Check(predicate);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkNodePredicateFunction)

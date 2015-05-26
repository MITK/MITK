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

#include "mitkTestingMacros.h"
#include <mitkTestingConfig.h>
#include <mitkTestFixture.h>

#include "mitkRdfNode.h"

class mitkRdfNodeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRdfNodeTestSuite);

  // List of Tests
  MITK_TEST(TestInvalidNode);
  MITK_TEST(TestUriNode);
  MITK_TEST(TestLiteralNode);
  MITK_TEST(TestLiteralNodeWithDataType);
  MITK_TEST(TestValueOfNode);
  MITK_TEST(TestTypeOfNode);
  MITK_TEST(TestDatatypeOfNode);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::RdfNode emptyNode;
  mitk::RdfUri base;

public:

  void setUp() override
  {
    emptyNode = mitk::RdfNode();
    base = mitk::RdfUri("http://mitk.org/wiki/MITK/data/BaseOntology.rdf#");
  }

  void tearDown() override
  {
  }

  // Test functions

  void TestInvalidNode()
  {
    mitk::RdfNode anotherInvalidNode("", mitk::RdfUri());
    anotherInvalidNode.SetType(mitk::RdfNode::NOTHING);
    CPPUNIT_ASSERT(emptyNode == anotherInvalidNode);
  }

  void TestUriNode()
  {
    mitk::RdfNode uriNode = mitk::RdfNode(base);
    emptyNode.SetValue(base.ToString());
    emptyNode.SetType(mitk::RdfNode::URI);
    emptyNode.SetDatatype(mitk::RdfUri());
    CPPUNIT_ASSERT(uriNode == emptyNode);
  }

  void TestLiteralNode()
  {
    mitk::RdfNode literalNode = mitk::RdfNode("Example");
    emptyNode.SetValue("Example");
    emptyNode.SetType(mitk::RdfNode::LITERAL);
    emptyNode.SetDatatype(mitk::RdfUri());
    CPPUNIT_ASSERT(literalNode == emptyNode);
  }

  void TestLiteralNodeWithDataType()
  {
    mitk::RdfNode literalNode = mitk::RdfNode("MyTest", mitk::RdfUri("xsd:string"));
    emptyNode.SetValue("MyTest");
    emptyNode.SetType(mitk::RdfNode::LITERAL);
    emptyNode.SetDatatype(mitk::RdfUri("xsd:string"));
    CPPUNIT_ASSERT(literalNode == emptyNode);
  }

  void TestValueOfNode()
  {
    mitk::RdfNode node(base);
    CPPUNIT_ASSERT(base.ToString().compare(node.GetValue()) == 0);
  }

  void TestTypeOfNode()
  {
    mitk::RdfNode node(base);
    CPPUNIT_ASSERT(node.GetType() == mitk::RdfNode::URI);
  }

  void TestDatatypeOfNode()
  {
    mitk::RdfNode literalNodeAgain = mitk::RdfNode("42", mitk::RdfUri("xsd:integer"));
    CPPUNIT_ASSERT(literalNodeAgain.GetDatatype() == mitk::RdfUri("xsd:integer"));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRdfNode)

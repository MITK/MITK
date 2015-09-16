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

#include "mitkRdfTriple.h"

class mitkRdfTripleTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRdfTripleTestSuite);

  // List of Tests
  MITK_TEST(TestEmptyTriple);
  MITK_TEST(TestObjectTriple);
  MITK_TEST(TestDataTriple);
  MITK_TEST(TestSubjectOfTriple);
  MITK_TEST(TestPredicateOfTriple);
  MITK_TEST(TestObjectOfTripleAsObject);
  MITK_TEST(TestObjectOfTripleAsData);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::RdfTriple emptyTriple;
  mitk::RdfUri seg;
  mitk::RdfUri title;

public:

  void setUp()
  {
    emptyTriple = mitk::RdfTriple();
    title = mitk::RdfUri("dcterms:title");
    seg = mitk::RdfUri("http://mitk.org/wiki/MITK/data/instance.rdf#s0001");
  }

  void tearDown()
  {
  }

  // Test functions

  void TestEmptyTriple()
  {
    mitk::RdfTriple anotherTriple = mitk::RdfTriple(
      mitk::RdfNode(), mitk::RdfNode(), mitk::RdfNode());
    CPPUNIT_ASSERT(emptyTriple == anotherTriple);
  }

  void TestObjectTriple()
  {
    mitk::RdfUri src = mitk::RdfUri("dcterms:source");
    mitk::RdfUri image = mitk::RdfUri("http://mitk.org/wiki/MITK/data/instance.rdf#i0012");
    mitk::RdfTriple objectTriple = mitk::RdfTriple(
      mitk::RdfNode(seg), mitk::RdfNode(src), mitk::RdfNode(image));
    emptyTriple.SetTripleSubject(seg);
    emptyTriple.SetTriplePredicate(src);
    emptyTriple.SetTripleObject(mitk::RdfNode(image));
    CPPUNIT_ASSERT(emptyTriple == objectTriple);
  }

  void TestDataTriple()
  {
    mitk::RdfTriple dataTriple = mitk::RdfTriple(
      mitk::RdfNode(seg), mitk::RdfNode(title), "TestLiver");
    emptyTriple.SetTripleSubject(seg);
    emptyTriple.SetTriplePredicate(title);
    emptyTriple.SetTripleObject("TestLiver");
    CPPUNIT_ASSERT(emptyTriple == dataTriple);
  }

  void TestSubjectOfTriple()
  {
    mitk::RdfTriple triple = mitk::RdfTriple(
      mitk::RdfNode(seg), mitk::RdfNode(title), "KidneyLeft");

    CPPUNIT_ASSERT(triple.GetTripleSubject() == mitk::RdfNode(seg));
  }

  void TestPredicateOfTriple()
  {
    mitk::RdfTriple triple = mitk::RdfTriple(
      mitk::RdfNode(seg), mitk::RdfNode(title), "KidneyRight");
    CPPUNIT_ASSERT(triple.GetTriplePredicate() == mitk::RdfNode(title));
  }

  void TestObjectOfTripleAsObject()
  {
    mitk::RdfNode image = mitk::RdfNode(
      mitk::RdfUri("http://mitk.org/wiki/MITK/data/instance.rdf#i0012"));
    mitk::RdfTriple triple = mitk::RdfTriple(
      mitk::RdfNode(seg), mitk::RdfNode(mitk::RdfUri("dcterms:source")), image);
    CPPUNIT_ASSERT(triple.GetTripleObject() == image);
  }

  void TestObjectOfTripleAsData()
  {
    mitk::RdfTriple triple = mitk::RdfTriple(
      mitk::RdfNode(seg), mitk::RdfNode(mitk::RdfUri("mitk:volumeInMl")), "450");
    CPPUNIT_ASSERT(triple.GetTripleObject() == mitk::RdfNode("450"));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRdfTriple)

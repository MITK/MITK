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

#include "mitkRdfStore.h"
#include <mitkIOUtil.h>

#include <fstream>

class mitkRdfStoreTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRdfStoreTestSuite);

  // List of Tests
  MITK_TEST(TestAddPrefix);
  MITK_TEST(TestAddTriple);
  MITK_TEST(TestRemoveTriple);
  MITK_TEST(TestQueryTriple);
  MITK_TEST(TestSaveStore);
  MITK_TEST(TestImportStore);

  MITK_TEST(ExecuteBooleanQuery_PatternHasSolution_ReturnsTrue);
  MITK_TEST(ExecuteBooleanQuery_PatternDoesNotHaveSolution_ReturnsFalse);
  MITK_TEST(ExecuteBooleanQuery_MalformedBooleanQuery_ThrowsException);
  MITK_TEST(ExecuteBooleanQuery_NonBooleanQuery_ThrowsException);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::RdfStore store;
  mitk::RdfTriple triple;

public:

  void setUp()
  {
    store.SetBaseUri(mitk::RdfUri("http://mitk.org/wiki/MITK/data/instances.rdf#"));
    store.AddPrefix("dcterms", mitk::RdfUri("http://purl.org/dc/terms/"));
    store.AddPrefix("mitk", mitk::RdfUri("http://mitk.org/wiki/MITK/data/BaseOntology.rdf#"));
    triple = mitk::RdfTriple(mitk::RdfNode(mitk::RdfUri("http://mitk.org/wiki/MITK/data/instances.rdf#i0012")),
      mitk::RdfNode(mitk::RdfUri("dcterms:title")), mitk::RdfNode("TestImage"));
  }

  void tearDown()
  {
    store.CleanUp();
  }

  // Test functions

  void TestAddPrefix()
  {
    mitk::RdfUri fma = mitk::RdfUri("http://www.bioontology.org/projects/ontologies/fma/");
    store.AddPrefix("fma", fma);
    std::map<std::string, mitk::RdfUri> testMap = store.GetPrefixes();
    CPPUNIT_ASSERT(testMap.find("fma") != testMap.end());
  }

  void TestAddTriple()
  {
    store.Add(triple);
    CPPUNIT_ASSERT(store.Contains(triple));
  }

  void TestRemoveTriple()
  {
    store.Add(triple);
    store.Remove(triple);
    CPPUNIT_ASSERT(!store.Contains(triple));
  }

  void TestQueryTriple()
  {
    store.Add(triple);
    std::string query = "SELECT ?x WHERE { ?x ?z ?y . }";
    mitk::RdfStore::ResultMap queryResult = store.ExecuteTupleQuery(query);
    std::list<mitk::RdfNode> list = queryResult["x"];
    CPPUNIT_ASSERT(triple.GetTripleSubject() == list.back());
  }

  void TestSaveStore()
  {
    store.Add(triple);

    std::ofstream tmpStream;
    const std::string tmpFileName = mitk::IOUtil::CreateTemporaryFile(tmpStream);
    store.Save(tmpFileName);
    std::ifstream in(tmpFileName);
    std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    MITK_INFO << s;
    tmpStream.close();

    std::size_t found = s.find(":i0012");
    CPPUNIT_ASSERT(found!=std::string::npos);
  }

  void TestImportStore()
  {
    std::ofstream tmpStream;
    const std::string tmpFileName = mitk::IOUtil::CreateTemporaryFile(tmpStream);
    const std::string strRDF = "@base <http://mitk.org/wiki/MITK/data/instances.rdf#> . @prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> . @prefix : <> . @prefix dcterms: <http://purl.org/dc/terms/> . @prefix mitk: <BaseOntology.rdf#> . @prefix owl: <http://www.w3.org/2002/07/owl#> . @prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> . @prefix xsd: <http://www.w3.org/2001/XMLSchema#> . :i0012 <dcterms:title> 'TestImage' .";
    tmpStream << strRDF;
    tmpStream.close();

    store.Import("file:"+tmpFileName);
    CPPUNIT_ASSERT(store.Contains(triple));
  }

  void ExecuteBooleanQuery_PatternHasSolution_ReturnsTrue(void)
  {
    store.Add(triple);

    const std::string query = "ASK { <http://mitk.org/wiki/MITK/data/instances.rdf#i0012> <dcterms:title> 'TestImage' }";

    CPPUNIT_ASSERT_EQUAL(true, store.ExecuteBooleanQuery(query));
  }

  void ExecuteBooleanQuery_PatternDoesNotHaveSolution_ReturnsFalse(void)
  {
    const std::string query = "ASK { <http://mitk.org/wiki/MITK/data/instances.rdf#i0012> <dcterms:title> 'TestImage' }";

    CPPUNIT_ASSERT_EQUAL(false, store.ExecuteBooleanQuery(query));
  }

  void ExecuteBooleanQuery_MalformedBooleanQuery_ThrowsException(void)
  {
    const std::string query = "ASK { <http://mitk.org/wiki/MITK/data/instances.rdf#i0012> <dcterms:title> TestImage }";

    CPPUNIT_ASSERT_THROW(store.ExecuteBooleanQuery(query), mitk::Exception);
  }

  void ExecuteBooleanQuery_NonBooleanQuery_ThrowsException(void)
  {
    const std::string query = "SELECT ?x ?y ?z WHERE { ?x ?y ?z }";

    CPPUNIT_ASSERT_THROW(store.ExecuteBooleanQuery(query), mitk::Exception);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkRdfStore)

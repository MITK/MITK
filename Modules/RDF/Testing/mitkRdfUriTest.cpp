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

#include "mitkRdfUri.h"

class mitkRdfUriTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRdfUriTestSuite);

  // List of Tests
  MITK_TEST(TestEmptyUri);
  MITK_TEST(TestValueOfUri);
  MITK_TEST(TestSetUri);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::RdfUri m_EmptyUri;
  std::string m_UriText;
  mitk::RdfUri m_Uri;

public:

  void setUp()
  {
    // normal URI
    m_UriText = "http://mitk.org/wiki/MITK/data/BaseOntology.rdf#";
    m_Uri = mitk::RdfUri(m_UriText);
  }

  void tearDown()
  {
  }

  // Test functions

  void TestEmptyUri()
  {
    mitk::RdfUri anotherEmptyUri("");
    CPPUNIT_ASSERT(m_EmptyUri == anotherEmptyUri);
  }

  void TestValueOfUri()
  {
    CPPUNIT_ASSERT(m_Uri.ToString().compare(m_UriText) == 0);
  }

  void TestSetUri()
  {
    mitk::RdfUri newUri;
    newUri.SetUri(m_UriText);
    CPPUNIT_ASSERT(newUri == m_Uri);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRdfUri)

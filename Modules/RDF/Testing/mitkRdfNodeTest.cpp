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
  // List of Tests
  CPPUNIT_TEST_SUITE(mitkRdfNodeTestSuite);

  MITK_TEST(TestDummy);

  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() override
  {
  }

  void tearDown() override
  {
  }

  // Test functions

  void TestDummy()
  {
    mitk::RdfNode node;
    CPPUNIT_ASSERT(node.dummy() == true);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkRdfNode)

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

//testing headers
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

//headers of IGT classes releated to the tested class
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkStandaloneDataStorage.h>

class mitkNavigationToolStorageDeserializerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNavigationToolStorageDeserializerTestSuite);
  MITK_TEST(TestInstantiationDeserializer);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/

public:
  /**@brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).*/
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void TestInstantiationDeserializer()
  {
  mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
  mitk::NavigationToolStorageDeserializer::Pointer testDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);
  CPPUNIT_ASSERT_MESSAGE("Testing instantiation of NavigationToolStorageDeserializer",testDeserializer.IsNotNull());
  }

};
MITK_TEST_SUITE_REGISTRATION(mitkNavigationToolStorageDeserializer)

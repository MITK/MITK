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

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
// std includes
#include <string>
// MITK includes
#include "mitkUIDGenerator.h"
#include <mitkLogMacros.h>
// VTK includes
#include <vtkDebugLeaks.h>

class mitkUIDGeneratorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkUIDGeneratorTestSuite);
  MITK_TEST(newGeneratorInstancesHeap_DifferentUIDs);
  MITK_TEST(multipleUIDsFromSameGenerator_DifferentUIDs);
  MITK_TEST(newGeneratorInstances_DifferentUIDs);
  MITK_TEST(severalGeneratorInstances_DifferentUIDs);
  CPPUNIT_TEST_SUITE_END();


private:
  std::string m_Uid1, m_Uid2;

public:
  void setUp()
  {
  }

  void tearDown()
  {
    m_Uid1 = "";
    m_Uid2 = "";
  }

  void newGeneratorInstancesHeap_DifferentUIDs()
  {
    auto uidGen1 = new mitk::UIDGenerator("UID_", 8);
    mitk::UIDGenerator *uidGen2 = uidGen1;
    m_Uid1 = uidGen1->GetUID();
    
    uidGen1 = new mitk::UIDGenerator("UID_", 8);
    
    m_Uid2 = uidGen1->GetUID();
    
    delete uidGen1;
    delete uidGen2;
    
    CPPUNIT_ASSERT_MESSAGE("First UIDs of two different generators are not allowed to be equal.", m_Uid1 != m_Uid2);
  }

  void multipleUIDsFromSameGenerator_DifferentUIDs()
  {
    auto uidGen = new mitk::UIDGenerator("UID_", 8);
    m_Uid1 = uidGen->GetUID();
    m_Uid2 = uidGen->GetUID();
    delete uidGen;
    CPPUNIT_ASSERT_MESSAGE("Testing two UIDs from the same generator. Different UIDs are not allowed to be equal", m_Uid1 != m_Uid2);
  }

  void newGeneratorInstances_DifferentUIDs()
  {
    mitk::UIDGenerator uidGen1("UID_", 8);
    m_Uid1 = uidGen1.GetUID();
    
    uidGen1 = mitk::UIDGenerator("UID_", 8);
    m_Uid2 = uidGen1.GetUID();
    
    CPPUNIT_ASSERT_MESSAGE("Testing two UIDs from new Instances. Different UIDs are not allowed to be equal", m_Uid1 != m_Uid2);
  }

  void severalGeneratorInstances_DifferentUIDs()
  {
    mitk::UIDGenerator uidGen1("UID_", 8);
    mitk::UIDGenerator uidGen2("UID_", 8);
    
    m_Uid1 = uidGen1.GetUID();
    m_Uid2 = uidGen2.GetUID();
    
    CPPUNIT_ASSERT_MESSAGE("Testing two UIDs from different Generators. Different UIDs are not allowed to be equal", m_Uid1 != m_Uid2);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkUIDGenerator)

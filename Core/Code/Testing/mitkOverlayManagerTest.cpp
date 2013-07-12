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

//MITK
#include "mitkTestingMacros.h"
#include "mitkRenderingTestHelper.h"
#include <Overlays/mitkOverlayManager.h>


class mitkOverlayManagerTestClass
{
public:
  static void TestSingleOverlayManager()
  {
    mitk::OverlayManager::Pointer OverlayManagerCreated = mitk::OverlayManager::New();

    mitk::OverlayManager::Pointer OverlayManager = mitk::OverlayManager::GetServiceInstance();
    MITK_TEST_CONDITION_REQUIRED( OverlayManager.IsNotNull(), "Microservice created and retrievable" );
    MITK_TEST_CONDITION_REQUIRED( OverlayManager.GetPointer() == OverlayManagerCreated.GetPointer(), "Correct OverlayManager Microservice" );
  }

  static void TestSeveralOverlayManagers()
  {
    mitk::OverlayManager::Pointer OverlayManagerOne = mitk::OverlayManager::New();
    std::string OverlayManagerOneID = OverlayManagerOne->GetID();

    mitk::OverlayManager::Pointer OverlayManagerTwo = mitk::OverlayManager::New();
    std::string OverlayManagerTwoID = OverlayManagerTwo->GetID();

    mitk::OverlayManager::Pointer OverlayManagerOneRetrieved = mitk::OverlayManager::GetServiceInstance(OverlayManagerOneID);
    MITK_TEST_CONDITION_REQUIRED( OverlayManagerOneRetrieved.IsNotNull(), "Microservice created and retrievable" );
    MITK_TEST_CONDITION_REQUIRED( OverlayManagerOneRetrieved.GetPointer() == OverlayManagerOne.GetPointer(), "Correct OverlayManager Microservice" );

    mitk::OverlayManager::Pointer OverlayManagerTwoRetrieved = mitk::OverlayManager::GetServiceInstance(OverlayManagerTwoID);
    MITK_TEST_CONDITION_REQUIRED( OverlayManagerTwoRetrieved.IsNotNull(), "Microservice created and retrievable" );
    MITK_TEST_CONDITION_REQUIRED( OverlayManagerTwoRetrieved.GetPointer() == OverlayManagerTwo.GetPointer(), "Correct OverlayManager Microservice" );
  }

  static void TestNotInstatiatedOverlayManager()
  {
    mitk::OverlayManager::Pointer OverlayManager = mitk::OverlayManager::GetServiceInstance();
    MITK_TEST_CONDITION_REQUIRED( OverlayManager.IsNull(), "No OverlayManager was instanciated, so there is none" );
  }
};

int mitkOverlayManagerTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkOverlayManagerTest");

  mitkOverlayManagerTestClass::TestNotInstatiatedOverlayManager();
  mitkOverlayManagerTestClass::TestSingleOverlayManager();
  mitkOverlayManagerTestClass::TestSeveralOverlayManagers();


  MITK_TEST_END();
}


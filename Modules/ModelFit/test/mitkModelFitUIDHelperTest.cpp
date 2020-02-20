/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <iostream>
#include "mitkTestingMacros.h"
#include "mitkDataNode.h"
#include "mitkImage.h"
#include "mitkStandaloneDataStorage.h"

#include "mitkModelFitUIDHelper.h"

int mitkModelFitUIDHelperTest(int  /*argc*/, char*[] /*argv[]*/)
{
  MITK_TEST_BEGIN("mitkModelFitUIDHelperTest")

  mitk::DataNode::Pointer node1 = mitk::DataNode::New();
  node1->SetData(mitk::Image::New());
  mitk::DataNode::Pointer node2 = mitk::DataNode::New();
  node2->SetData(mitk::Image::New());

  mitk::DataNode* nullNode = nullptr;
  mitk::BaseData* nullData = nullptr;

  MITK_TEST_FOR_EXCEPTION(mitk::Exception, mitk::EnsureModelFitUID(nullNode));
  MITK_TEST_CONDITION_REQUIRED(mitk::CheckModelFitUID(nullNode,"testUID") == false,
    "Testing if CheckModelFitUID fails of null pointer is passed.");
  MITK_TEST_FOR_EXCEPTION(mitk::Exception, mitk::EnsureModelFitUID(nullData));
  MITK_TEST_CONDITION_REQUIRED(mitk::CheckModelFitUID(nullData, "testUID") == false,
    "Testing if CheckModelFitUID fails of null pointer is passed.");

  mitk::NodeUIDType uid = mitk::EnsureModelFitUID(node1);

  MITK_TEST_CONDITION_REQUIRED(mitk::CheckModelFitUID(node1,"testUID") == false,
    "Testing if CheckModelFitUID fails of wrong UID is passed.");
  MITK_TEST_CONDITION_REQUIRED(mitk::CheckModelFitUID(node1, uid) == true,
    "Testing if CheckModelFitUID succeeds if correct UID is passed.");

  mitk::NodeUIDType uid2ndCall = mitk::EnsureModelFitUID(node1);

  MITK_TEST_CONDITION_REQUIRED(uid == uid2ndCall,
    "Testing if EnsureModelFitUID does not create new UIDs on multiple calls.");

  mitk::NodeUIDType uid2 = mitk::EnsureModelFitUID(node2);

  MITK_TEST_CONDITION_REQUIRED(uid != uid2,
    "Testing if EnsureModelFitUID does create different UIDs for different nodes.");

  mitk::StandaloneDataStorage::Pointer storage = mitk::StandaloneDataStorage::New();
  storage->Add(node1);
  storage->Add(node2);

  MITK_TEST_CONDITION_REQUIRED(node1 == GetNodeByModelFitUID(storage,uid),
    "Testing if GetNodeByModelFitUID finds node 1.");

  MITK_TEST_CONDITION_REQUIRED(node2 == GetNodeByModelFitUID(storage,uid2),
    "Testing if GetNodeByModelFitUID finds node 2.");

  MITK_TEST_CONDITION_REQUIRED(GetNodeByModelFitUID(storage,"unkown_uid").IsNull(),
    "Testing if GetNodeByModelFitUID returns NULL for unkown UID.");

  MITK_TEST_END()
}

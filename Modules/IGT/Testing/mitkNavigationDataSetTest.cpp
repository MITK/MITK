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
#include "mitkNavigationData.h"
#include "mitkNavigationDataSet.h"

static void TestEmptySet()
{
  mitk::NavigationDataSet::Pointer navigationDataSet = mitk::NavigationDataSet::New(1);

  MITK_TEST_CONDITION_REQUIRED(! navigationDataSet->GetNavigationDataForIndex(0,0) , "Trying to get non-existant NavigationData by index should return false.");
  //MITK_TEST_CONDITION_REQUIRED(! navigationDataSet->GetNavigationDataBeforeTimestamp(0, 100), "Trying to get non-existant NavigationData by timestamp should return false.")
}

static void TestSetAndGet()
{
  mitk::NavigationDataSet::Pointer navigationDataSet = mitk::NavigationDataSet::New(2);

  mitk::NavigationData::Pointer nd11 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd12 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd13 = mitk::NavigationData::New();
  nd12->SetIGTTimeStamp(1);
  nd12->SetIGTTimeStamp(2);

  mitk::NavigationData::Pointer nd21 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd22 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd23 = mitk::NavigationData::New();
  nd22->SetIGTTimeStamp(1);

  // First set, Timestamp = 0
  std::vector<mitk::NavigationData::Pointer> step1;
  step1.push_back(nd11);
  step1.push_back(nd21);

  // Second set, Timestamp = 1
  std::vector<mitk::NavigationData::Pointer> step2;
  step2.push_back(nd12);
  step2.push_back(nd22);

  //Third set, Timestamp invalid ()
  std::vector<mitk::NavigationData::Pointer> step3;
  step3.push_back(nd13);
  step3.push_back(nd23);

  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->AddNavigationDatas(step1),
    "Adding a valid first set, should be successful.");
  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->AddNavigationDatas(step2),
    "Adding a valid second set, should be successful.");
  MITK_TEST_CONDITION_REQUIRED(!(navigationDataSet->AddNavigationDatas(step3)),
    "Adding an invalid third set, should be unsusuccessful.");

  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->GetNavigationDataForIndex(0, 0) == nd11,
    "First NavigationData object for tool 0 should be the same as added previously.");
  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->GetNavigationDataForIndex(0, 1) == nd21,
    "Second NavigationData object for tool 0 should be the same as added previously.");
  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->GetNavigationDataForIndex(1, 0) == nd12,
    "First NavigationData object for tool 0 should be the same as added previously.");
  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->GetNavigationDataForIndex(1, 1) == nd22,
    "Second NavigationData object for tool 0 should be the same as added previously.");

  std::vector<mitk::NavigationData::Pointer> result = navigationDataSet->GetTimeStep(1);
  MITK_TEST_CONDITION_REQUIRED(nd12 == result[0],"Comparing returned datas from GetTimeStep().");
  MITK_TEST_CONDITION_REQUIRED(nd22 == result[1],"Comparing returned datas from GetTimeStep().");

  result = navigationDataSet->GetDataStreamForTool(1);
  MITK_TEST_CONDITION_REQUIRED(nd21 == result[0],"Comparing returned datas from GetStreamForTool().");
  MITK_TEST_CONDITION_REQUIRED(nd22 == result[1],"Comparing returned datas from GetStreamForTool().");
}

/**
*
*/
int mitkNavigationDataSetTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataSet");

  TestEmptySet();
  TestSetAndGet();

  MITK_TEST_END();
}
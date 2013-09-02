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
  MITK_TEST_CONDITION_REQUIRED(! navigationDataSet->GetNavigationDataBeforeTimestamp(0, 100), "Trying to get non-existant NavigationData by timestamp should return false.")
}

static void TestSetGetByIndex()
{
  mitk::NavigationDataSet::Pointer navigationDataSet = mitk::NavigationDataSet::New(2);

  mitk::NavigationData::Pointer nd11 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd12 = mitk::NavigationData::New(); nd12->SetIGTTimeStamp(1);
  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();

  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->InsertNavigationData(0, nd11),
                               "Adding NavigationData for tool 0 should be successfull.");
  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->InsertNavigationData(0, nd12),
                               "Adding second NavigationData for tool 0 should be successfull.");
  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->InsertNavigationData(1, nd2),
                               "Adding NavigationData for tool 1 should be successfull.");

  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->GetNavigationDataForIndex(0, 0) == nd11,
                               "First NavigationData object for tool 0 should be the same as added previously.");
  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->GetNavigationDataForIndex(0, 1) == nd12,
                               "Second NavigationData object for tool 0 should be the same as added previously.");
  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->GetNavigationDataForIndex(1, 0) == nd2,
                               "NavigationData object for tool 1 should be the same as added previously.");
}

static void TestSetGetBeforeTimestamp()
{
  mitk::NavigationDataSet::Pointer navigationDataSet = mitk::NavigationDataSet::New(2);

  mitk::NavigationData::Pointer nd11 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd12 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd21 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd22 = mitk::NavigationData::New();

  nd11->SetIGTTimeStamp(1.3); nd12->SetIGTTimeStamp(3.5);
  nd21->SetIGTTimeStamp(1.3); nd22->SetIGTTimeStamp(3.5);

  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->InsertNavigationData(0, nd11) &&
                               navigationDataSet->InsertNavigationData(0, nd12) &&
                               navigationDataSet->InsertNavigationData(1, nd21) &&
                               navigationDataSet->InsertNavigationData(1, nd22),
                               "Adding NavigationData should be no problem now.");

  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->GetNavigationDataBeforeTimestamp(0, 2) == nd11,
                               "First navigation data object should be returned when timestamp '2' is given.");
  MITK_TEST_CONDITION_REQUIRED(navigationDataSet->GetNavigationDataBeforeTimestamp(0, 3.5) == nd12,
                               "Second navigation data object should be returned when timestamp '3.5' is given.");
}

/**
  *
  */
int mitkNavigationDataSetTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataSet");

  TestEmptySet();
  TestSetGetByIndex();
  TestSetGetBeforeTimestamp();

  MITK_TEST_END();
}

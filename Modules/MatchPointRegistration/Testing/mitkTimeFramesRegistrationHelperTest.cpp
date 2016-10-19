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
#include "mitkTestFixture.h"

#include "mitkTimeFramesRegistrationHelper.h"

class mitkTimeFramesRegistrationHelperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkTimeFramesRegistrationHelperTestSuite);
  // Test the append method
  MITK_TEST(SetAllowUndefPixels_GetAllowUndefPixels);
  MITK_TEST(SetPaddingValue_GetPaddingValue);
  MITK_TEST(SetErrorValue_GetErrorValue);
  MITK_TEST(SetAllowUnregPixels_GetAllowUnregPixels);
  MITK_TEST(SetInterpolatorType_GetInterpolatorType);
  MITK_TEST(Set_Get_Clear_IgnoreList);
  CPPUNIT_TEST_SUITE_END();
private:
  mitk::TimeFramesRegistrationHelper::Pointer frameRegHelper;
  mitk::TimeFramesRegistrationHelper::IgnoreListType ignoreList;

public:
  void setUp() override
  {
    frameRegHelper = mitk::TimeFramesRegistrationHelper::New();
    ignoreList.clear();
    ignoreList.push_back(2);
    ignoreList.push_back(13);
  }

  void tearDown() override
  {
  }

  void SetAllowUndefPixels_GetAllowUndefPixels()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check getter on default value", true,
                                 frameRegHelper->GetAllowUndefPixels());
    frameRegHelper->SetAllowUndefPixels(false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check getter on changed value", false,
                                 frameRegHelper->GetAllowUndefPixels());
  }

  void SetPaddingValue_GetPaddingValue()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check getter on default value", 0.0,
                                 frameRegHelper->GetPaddingValue());
    frameRegHelper->SetPaddingValue(11);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check getter on changed value", 11.0,
                                 frameRegHelper->GetPaddingValue());
  }

  void SetAllowUnregPixels_GetAllowUnregPixels()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check getter on default value", true,
                                 frameRegHelper->GetAllowUnregPixels());
    frameRegHelper->SetAllowUnregPixels(false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check getter on changed value", false,
                                 frameRegHelper->GetAllowUnregPixels());
  }

  void SetErrorValue_GetErrorValue()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check getter on default value", 0.0, frameRegHelper->GetErrorValue());
    frameRegHelper->SetErrorValue(15);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check getter on changed value", 15.0,
                                 frameRegHelper->GetErrorValue());
  }

  void SetInterpolatorType_GetInterpolatorType()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check getter on default value", mitk::ImageMappingInterpolator::Linear,
                                 frameRegHelper->GetInterpolatorType());
    frameRegHelper->SetInterpolatorType(mitk::ImageMappingInterpolator::NearestNeighbor);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check getter on changed value",
                                 mitk::ImageMappingInterpolator::NearestNeighbor, frameRegHelper->GetInterpolatorType());
  }

  void Set_Get_Clear_IgnoreList()
  {
    CPPUNIT_ASSERT(frameRegHelper->GetIgnoreList().empty());

    itk::ModifiedTimeType mtime = frameRegHelper->GetMTime();
    frameRegHelper->SetIgnoreList(ignoreList);
    CPPUNIT_ASSERT(mtime < frameRegHelper->GetMTime());
    CPPUNIT_ASSERT(ignoreList == frameRegHelper->GetIgnoreList());

    mtime = frameRegHelper->GetMTime();
    frameRegHelper->ClearIgnoreList();
    CPPUNIT_ASSERT(mtime < frameRegHelper->GetMTime());
    CPPUNIT_ASSERT(frameRegHelper->GetIgnoreList().empty());
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkTimeFramesRegistrationHelper)
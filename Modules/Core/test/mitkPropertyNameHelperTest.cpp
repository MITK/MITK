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

#include "mitkPropertyNameHelper.h"
#include "mitkStringProperty.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <limits>

class mitkPropertyNameHelperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyNameHelperTestSuite);
  // Test the append method
  MITK_TEST(GeneratePropertyNameForDICOMTag);
  MITK_TEST(GetDefaultDICOMTagsOfInterest);
  MITK_TEST(GetBackwardsCompatibleDICOMProperty);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::PropertyList::Pointer _propList;
  std::string oldStudyName;
  std::string oldSeriesName;

public:
  void setUp() override
  {
    _propList = mitk::PropertyList::New();
    oldSeriesName = "dicom.study.SeriesDescription";
    oldStudyName = "dicom.study.StudyDescription";

    _propList->SetStringProperty(oldSeriesName.c_str(), "old_series");
    _propList->SetStringProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x103e).c_str(), "new_series");

    _propList->SetStringProperty(oldStudyName.c_str(), "old_study");
  }

  void tearDown() override {}
  void GeneratePropertyNameForDICOMTag()
  {
    std::string result = mitk::GeneratePropertyNameForDICOMTag(0x0018, 0x0080);
    MITK_TEST_CONDITION_REQUIRED(result == "DICOM.0018.0080",
                                 "Testing GeneratePropertyNameForDICOMTag(mitk::DICOMTag(0x0018, 0x0080)");

    result = mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x001a);
    MITK_TEST_CONDITION_REQUIRED(result == "DICOM.0008.001A",
                                 "Testing GeneratePropertyNameForDICOMTag(mitk::DICOMTag(0x0008, 0x001a)");
  }

  void GetBackwardsCompatibleDICOMProperty()
  {
    std::string result = "";
    bool check = mitk::GetBackwardsCompatibleDICOMProperty(0x0008, 0x1030, oldStudyName, _propList, result);
    CPPUNIT_ASSERT_MESSAGE("Testing GetBackwardsCompatibleDICOMProperty. Only deprecated name is existing.", check);
    CPPUNIT_ASSERT_MESSAGE("Testing returned property value. Only deprecated name is existing.", result == "old_study");

    check = mitk::GetBackwardsCompatibleDICOMProperty(0x0008, 0x103e, oldSeriesName, _propList, result);
    CPPUNIT_ASSERT_MESSAGE("Testing GetBackwardsCompatibleDICOMProperty. Only deprecated name does not exist.", check);
    CPPUNIT_ASSERT_MESSAGE("Testing returned property value. Only deprecated name is existing.",
                           result == "new_series");

    check = mitk::GetBackwardsCompatibleDICOMProperty(0x0001, 0x0001, "unkown_old_name", _propList, result);
    CPPUNIT_ASSERT_MESSAGE("Testing GetBackwardsCompatibleDICOMProperty. Only deprecated name does not exist.", !check);
  }

  void GetDefaultDICOMTagsOfInterest() {}
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyNameHelper)

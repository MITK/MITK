/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPropertyNameHelper.h>
#include <mitkStringProperty.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <limits>

class mitkPropertyNameHelperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyNameHelperTestSuite);
  // Test the append method
  MITK_TEST(GeneratePropertyNameForDICOMTag);
  MITK_TEST(GetDefaultDICOMTagsOfInterest);
  MITK_TEST(GetDICOMPropertyValue);
  MITK_TEST(GetBackwardsCompatibleDICOMPropertyValue);

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

    // A DICOM-tag property stored as TemporoSpatialStringProperty (the type
    // DICOM tags actually use). GetStringProperty cannot resolve this; the
    // GetValueAsString-based helpers can.
    _propList->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str(),
                           mitk::TemporoSpatialStringProperty::New("1.2.3.4"));
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

  void GetDICOMPropertyValue()
  {
    std::string result = "";

    // Resolves the standardized tag-named property.
    bool check = mitk::GetDICOMPropertyValue(0x0008, 0x103e, _propList, result);
    CPPUNIT_ASSERT_MESSAGE("GetDICOMPropertyValue finds the standardized tag property.", check);
    CPPUNIT_ASSERT_MESSAGE("GetDICOMPropertyValue returns its value.", result == "new_series");

    // Must resolve a TemporoSpatialStringProperty (the type DICOM tags use);
    // a plain GetStringProperty would silently miss it. This is the trap the
    // helper exists to avoid.
    check = mitk::GetDICOMPropertyValue(0x0020, 0x000e, _propList, result);
    CPPUNIT_ASSERT_MESSAGE("GetDICOMPropertyValue resolves a TemporoSpatialStringProperty.", check);
    CPPUNIT_ASSERT_MESSAGE("GetDICOMPropertyValue returns the TemporoSpatial value.", result == "1.2.3.4");

    // Does NOT consult the old naming style: a tag present only under the
    // deprecated name is not found.
    check = mitk::GetDICOMPropertyValue(0x0008, 0x1030, _propList, result);
    CPPUNIT_ASSERT_MESSAGE("GetDICOMPropertyValue ignores deprecated-only properties.", !check);
  }

  void GetBackwardsCompatibleDICOMPropertyValue()
  {
    std::string result = "";
    bool check = mitk::GetBackwardsCompatibleDICOMPropertyValue(0x0008, 0x1030, oldStudyName, _propList, result);
    CPPUNIT_ASSERT_MESSAGE("Testing GetBackwardsCompatibleDICOMPropertyValue. Only deprecated name is existing.", check);
    CPPUNIT_ASSERT_MESSAGE("Testing returned property value. Only deprecated name is existing.", result == "old_study");

    check = mitk::GetBackwardsCompatibleDICOMPropertyValue(0x0008, 0x103e, oldSeriesName, _propList, result);
    CPPUNIT_ASSERT_MESSAGE("Testing GetBackwardsCompatibleDICOMPropertyValue. Only deprecated name does not exist.", check);
    CPPUNIT_ASSERT_MESSAGE("Testing returned property value. Only deprecated name is existing.",
                           result == "new_series");

    check = mitk::GetBackwardsCompatibleDICOMPropertyValue(0x0001, 0x0001, "unkown_old_name", _propList, result);
    CPPUNIT_ASSERT_MESSAGE("Testing GetBackwardsCompatibleDICOMPropertyValue. Only deprecated name does not exist.", !check);
  }

  void GetDefaultDICOMTagsOfInterest() {}
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyNameHelper)

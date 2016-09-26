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

#include "mitkDICOMTagPath.h"

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

class mitkDICOMTagPathTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMTagPathTestSuite);

  MITK_TEST(DICOMTagPathToPropertRegEx);
  MITK_TEST(DICOMTagPathToPersistenceKeyRegEx);
  MITK_TEST(DICOMTagPathToPersistenceKeyTemplate);
  MITK_TEST(DICOMTagPathToPersistenceNameTemplate);
  MITK_TEST(DICOMTagPathToDCMTKSearchPath);
  MITK_TEST(PropertyNameToDICOMTagPath);
  MITK_TEST(DICOMTagPathToPropertyName);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::DICOMTagPath simplePath;
  mitk::DICOMTagPath deepPath;
  mitk::DICOMTagPath deepPath_withAnyElement;
  mitk::DICOMTagPath deepPath_withAnySelection;
  mitk::DICOMTagPath deepPath_withSelection;
  mitk::DICOMTagPath verydeepPath;

  mitk::DICOMTagPath emptyPath;

public:

  void setUp() override
  {
    simplePath.AddElement(0x0010, 0x0010);

    deepPath.AddElement(0x0010, 0x0011);
    deepPath.AddElement(0x0020, 0x0022);
    deepPath.AddElement(0x0030, 0x0033);

    deepPath_withAnyElement.AddElement(0x0010, 0x0011);
    deepPath_withAnyElement.AddAnyElement();
    deepPath_withAnyElement.AddElement(0x0030, 0x0033);

    deepPath_withAnySelection.AddElement(0x0010, 0x0011);
    deepPath_withAnySelection.AddAnySelection(0x0020, 0x0022);
    deepPath_withAnySelection.AddElement(0x0030, 0x0033);

    deepPath_withSelection.AddElement(0x0010, 0x0011);
    deepPath_withSelection.AddSelection(0x0020, 0x0022, 6);
    deepPath_withSelection.AddElement(0x0030, 0x0033);

    verydeepPath.AddAnySelection(0x0010, 0x0011);
    verydeepPath.AddAnyElement();
    verydeepPath.AddElement(0x0030, 0x0033);
    verydeepPath.AddSelection(0x0040, 0x0044, 4);
    verydeepPath.AddElement(0x0050, 0x0055);
  }

  void tearDown() override
  {
  }

  void DICOMTagPathToPropertRegEx()
  {
    std::string result = mitk::DICOMTagPathToPropertRegEx(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertRegEx() with '(0010,0010)'",  result, std::string("DICOM\\.0010\\.0010"));
    result = mitk::DICOMTagPathToPropertRegEx(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertRegEx() with '(0010,0010).(0020,0022).(0030,0033)'", result, std::string("DICOM\\.0010\\.0011\\.0020\\.0022\\.0030\\.0033"));
    result = mitk::DICOMTagPathToPropertRegEx(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertRegEx() with '(0010,0010).*.(0030,0033)'", result, std::string("DICOM\\.0010\\.0011\\.(\\d{4})\\.(\\d{4})\\.0030\\.0033"));
    result = mitk::DICOMTagPathToPropertRegEx(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertRegEx() with '(0010,0010).(0020,0022)[*].(0030,0033)'", result, std::string("DICOM\\.0010\\.0011\\.0020\\.0022\\.\\[(\\d*)\\]\\.0030\\.0033"));
    result = mitk::DICOMTagPathToPropertRegEx(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertRegEx() with '(0010,0010).(0020,0022)[6].(0030,0033)'", result, std::string("DICOM\\.0010\\.0011\\.0020\\.0022\\.\\[6\\]\\.0030\\.0033"));
    result = mitk::DICOMTagPathToPropertRegEx(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertRegEx() with '(0010,0010)[*].*.(0030,0033).(0040,0044)[4].(0050,0055)'", result, std::string("DICOM\\.0010\\.0011\\.\\[(\\d*)\\]\\.(\\d{4})\\.(\\d{4})\\.0030\\.0033\\.0040\\.0044\\.\\[4\\]\\.0050\\.0055"));
  }


  void DICOMTagPathToPersistenceKeyRegEx()
  {
    std::string result = mitk::DICOMTagPathToPersistenceKeyRegEx(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0010)'", result, std::string("DICOM_0010_0010"));
    result = mitk::DICOMTagPathToPersistenceKeyRegEx(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0010).(0020,0022).(0030,0033)'", result, std::string("DICOM_0010_0011_0020_0022_0030_0033"));
    result = mitk::DICOMTagPathToPersistenceKeyRegEx(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0010).*.(0030,0033)'", result, std::string("DICOM_0010_0011_(\\d{4})_(\\d{4})_0030_0033"));
    result = mitk::DICOMTagPathToPersistenceKeyRegEx(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0010).(0020,0022)[*].(0030,0033)'", result, std::string("DICOM_0010_0011_0020_0022_\\[(\\d*)\\]_0030_0033"));
    result = mitk::DICOMTagPathToPersistenceKeyRegEx(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0010).(0020,0022)[6].(0030,0033)'", result, std::string("DICOM_0010_0011_0020_0022_\\[6\\]_0030_0033"));
    result = mitk::DICOMTagPathToPersistenceKeyRegEx(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0010)[*].*.(0030,0033).(0040,0044)[4].(0050,0055)'", result, std::string("DICOM_0010_0011_\\[(\\d*)\\]_(\\d{4})_(\\d{4})_0030_0033_0040_0044_\\[4\\]_0050_0055"));
  }

  void DICOMTagPathToPersistenceKeyTemplate()
  {
    std::string result = mitk::DICOMTagPathToPersistenceKeyTemplate(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0010)'", result, std::string("DICOM_0010_0010"));
    result = mitk::DICOMTagPathToPersistenceKeyTemplate(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0010).(0020,0022).(0030,0033)'", result, std::string("DICOM_0010_0011_0020_0022_0030_0033"));
    result = mitk::DICOMTagPathToPersistenceKeyTemplate(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0010).*.(0030,0033)'", result, std::string("DICOM_0010_0011_$1_$2_0030_0033"));
    result = mitk::DICOMTagPathToPersistenceKeyTemplate(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0010).(0020,0022)[*].(0030,0033)'", result, std::string("DICOM_0010_0011_0020_0022_[$1]_0030_0033"));
    result = mitk::DICOMTagPathToPersistenceKeyTemplate(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0010).(0020,0022)[6].(0030,0033)'", result, std::string("DICOM_0010_0011_0020_0022_[6]_0030_0033"));
    result = mitk::DICOMTagPathToPersistenceKeyTemplate(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0010)[*].*.(0030,0033).(0040,0044)[4].(0050,0055)'", result, std::string("DICOM_0010_0011_[$1]_$2_$3_0030_0033_0040_0044_[4]_0050_0055"));
  }

  void DICOMTagPathToPersistenceNameTemplate()
  {
    std::string result = mitk::DICOMTagPathToPersistenceNameTemplate(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0010)'", result, std::string("DICOM.0010.0010"));
    result = mitk::DICOMTagPathToPersistenceNameTemplate(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0010).(0020,0022).(0030,0033)'", result, std::string("DICOM.0010.0011.0020.0022.0030.0033"));
    result = mitk::DICOMTagPathToPersistenceNameTemplate(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0010).*.(0030,0033)'", result, std::string("DICOM.0010.0011.$1.$2.0030.0033"));
    result = mitk::DICOMTagPathToPersistenceNameTemplate(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0010).(0020,0022)[*].(0030,0033)'", result, std::string("DICOM.0010.0011.0020.0022.[$1].0030.0033"));
    result = mitk::DICOMTagPathToPersistenceNameTemplate(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0010).(0020,0022)[6].(0030,0033)'", result, std::string("DICOM.0010.0011.0020.0022.[6].0030.0033"));
    result = mitk::DICOMTagPathToPersistenceNameTemplate(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0010)[*].*.(0030,0033).(0040,0044)[4].(0050,0055)'", result, std::string("DICOM.0010.0011.[$1].$2.$3.0030.0033.0040.0044.[4].0050.0055"));
  }

  void DICOMTagPathToDCMTKSearchPath()
  {
    std::string result = mitk::DICOMTagPathToDCMTKSearchPath(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToDCMTKSearchPath() with '(0010,0010)'", result, std::string("(0010,0010)"));
    result = mitk::DICOMTagPathToDCMTKSearchPath(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToDCMTKSearchPath() with '(0010,0011).(0020,0022).(0030,0033)'", result, std::string("(0010,0011).(0020,0022).(0030,0033)"));
    CPPUNIT_ASSERT_THROW(mitk::DICOMTagPathToDCMTKSearchPath(deepPath_withAnyElement), mitk::Exception);
    result = mitk::DICOMTagPathToDCMTKSearchPath(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToDCMTKSearchPath() with '(0010,0011).(0020,0022)[*].(0030,0033)'", result, std::string("(0010,0011).(0020,0022)[*].(0030,0033)"));
    result = mitk::DICOMTagPathToDCMTKSearchPath(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToDCMTKSearchPath() with '(0010,0011).(0020,0022)[6].(0030,0033)'", result, std::string("(0010,0011).(0020,0022)[6].(0030,0033)"));
    CPPUNIT_ASSERT_THROW(mitk::DICOMTagPathToDCMTKSearchPath(verydeepPath), mitk::Exception);
  }

  void PropertyNameToDICOMTagPath()
  {
    mitk::DICOMTagPath result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0010");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0010)'", simplePath, result);
    result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0011.0020.0022.0030.0033");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0010).(0020,0022).(0030,0033)'", deepPath, result);
    result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0011.*.0030.0033");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0010).*.(0030,0033)'", deepPath_withAnyElement, result);
    result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0011.0020.0022.[*].0030.0033");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0010).(0020,0022)[*].(0030,0033)'", deepPath_withAnySelection, result);
    result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0011.0020.0022.[6].0030.0033");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0010).(0020,0022)[6].(0030,0033)'", deepPath_withSelection, result);
    result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0011.[*].*.0030.0033.0040.0044.[4].0050.0055");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0010)[*].*.(0030,0033).(0040,0044)[4].(0050,0055)'", verydeepPath, result);

    result = mitk::PropertyNameToDICOMTagPath("WRONG.0010.0011.0020.0022.0030.0033");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with wrong path", emptyPath, result);
  }

  void DICOMTagPathToPropertyName()
  {
    std::string result = mitk::DICOMTagPathToPropertyName(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0010)'", result, std::string("DICOM.0010.0010"));
    result = mitk::DICOMTagPathToPropertyName(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0010).(0020,0022).(0030,0033)'", result, std::string("DICOM.0010.0011.0020.0022.0030.0033"));
    result = mitk::DICOMTagPathToPropertyName(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0010).*.(0030,0033)'", result, std::string("DICOM.0010.0011.*.0030.0033"));
    result = mitk::DICOMTagPathToPropertyName(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0010).(0020,0022)[*].(0030,0033)'", result, std::string("DICOM.0010.0011.0020.0022.[*].0030.0033"));
    result = mitk::DICOMTagPathToPropertyName(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0010).(0020,0022)[6].(0030,0033)'", result, std::string("DICOM.0010.0011.0020.0022.[6].0030.0033"));
    result = mitk::DICOMTagPathToPropertyName(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0010)[*].*.(0030,0033).(0040,0044)[4].(0050,0055)'", result, std::string("DICOM.0010.0011.[*].*.0030.0033.0040.0044.[4].0050.0055"));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMTagPath)

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

#include <regex>

class mitkDICOMTagPathTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMTagPathTestSuite);

  MITK_TEST(DICOMTagPathToPropertyRegEx);
  MITK_TEST(DICOMTagPathToPersistenceKeyRegEx);
  MITK_TEST(DICOMTagPathToPersistenceKeyTemplate);
  MITK_TEST(DICOMTagPathToPersistenceNameTemplate);
  MITK_TEST(DICOMTagPathToDCMTKSearchPath);
  MITK_TEST(PropertyNameToDICOMTagPath);
  MITK_TEST(DICOMTagPathToPropertyName);
  MITK_TEST(ExecutePropertyRegEx);

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
    simplePath.AddElement(0x0010, 0x0011);

    deepPath.AddElement(0x0010, 0x0011);
    deepPath.AddElement(0x0020, 0x0022);
    deepPath.AddElement(0x003A, 0x0033);

    deepPath_withAnyElement.AddElement(0x0010, 0x0011);
    deepPath_withAnyElement.AddAnyElement();
    deepPath_withAnyElement.AddElement(0x003a, 0x003f);

    deepPath_withAnySelection.AddElement(0x0010, 0x0011);
    deepPath_withAnySelection.AddAnySelection(0x002B, 0x002E);
    deepPath_withAnySelection.AddElement(0x0030, 0x0033);

    deepPath_withSelection.AddElement(0x0010, 0x0011);
    deepPath_withSelection.AddSelection(0x0020, 0x0022, 6);
    deepPath_withSelection.AddElement(0x003b, 0x003e);

    verydeepPath.AddAnySelection(0x0010, 0x0011);
    verydeepPath.AddAnyElement();
    verydeepPath.AddElement(0x0030, 0x0033);
    verydeepPath.AddSelection(0x004c, 0x004d, 4);
    verydeepPath.AddElement(0x0050, 0x0055);
  }

  void tearDown() override
  {
  }

  void DICOMTagPathToPropertyRegEx()
  {
    std::string result = mitk::DICOMTagPathToPropertyRegEx(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyRegEx() with '(0010,0011)'", std::string("DICOM\\.0010\\.0011"), result);
    result = mitk::DICOMTagPathToPropertyRegEx(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyRegEx() with '(0010,0011).(0020,0022).(003A,0033)'", std::string("DICOM\\.0010\\.0011\\.0020\\.0022\\.(003a|003A)\\.0033"), result);
    result = mitk::DICOMTagPathToPropertyRegEx(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyRegEx() with '(0010,0011).*.(003a,003f)'", std::string("DICOM\\.0010\\.0011\\.([A-Fa-f\\d]{4})\\.([A-Fa-f\\d]{4})\\.(003a|003A)\\.(003f|003F)"), result);
    result = mitk::DICOMTagPathToPropertyRegEx(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyRegEx() with '(0010,0011).(002B,002E)[*].(0030,0033)'", std::string("DICOM\\.0010\\.0011\\.(002b|002B)\\.(002e|002E)\\.\\[(\\d*)\\]\\.0030\\.0033"), result);
    result = mitk::DICOMTagPathToPropertyRegEx(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyRegEx() with '(0010,0011).(0020,0022)[6].(003b,003e)'", std::string("DICOM\\.0010\\.0011\\.0020\\.0022\\.\\[6\\]\\.(003b|003B)\\.(003e|003E)"), result);
    result = mitk::DICOMTagPathToPropertyRegEx(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyRegEx() with '(0010,0011)[*].*.(0030,0033).(004c,004d)[4].(0050,0055)'", std::string("DICOM\\.0010\\.0011\\.\\[(\\d*)\\]\\.([A-Fa-f\\d]{4})\\.([A-Fa-f\\d]{4})\\.0030\\.0033\\.(004c|004C)\\.(004d|004D)\\.\\[4\\]\\.0050\\.0055"), result);
  }

  void DICOMTagPathToPersistenceKeyRegEx()
  {
    std::string result = mitk::DICOMTagPathToPersistenceKeyRegEx(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0011)'", std::string("DICOM_0010_0011"), result);
    result = mitk::DICOMTagPathToPersistenceKeyRegEx(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0011).(0020,0022).(003A,0033)'", std::string("DICOM_0010_0011_0020_0022_(003a|003A)_0033"), result);
    result = mitk::DICOMTagPathToPersistenceKeyRegEx(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0011).*.(003a,003f)'", std::string("DICOM_0010_0011_([A-Fa-f\\d]{4})_([A-Fa-f\\d]{4})_(003a|003A)_(003f|003F)"), result);
    result = mitk::DICOMTagPathToPersistenceKeyRegEx(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0011).(002B,002E)[*].(0030,0033)'", std::string("DICOM_0010_0011_(002b|002B)_(002e|002E)_\\[(\\d*)\\]_0030_0033"), result);
    result = mitk::DICOMTagPathToPersistenceKeyRegEx(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0011).(0020,0022)[6].(003b,003e)'", std::string("DICOM_0010_0011_0020_0022_\\[6\\]_(003b|003B)_(003e|003E)"), result);
    result = mitk::DICOMTagPathToPersistenceKeyRegEx(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyRegEx() with '(0010,0011)[*].*.(0030,0033).(004c,004d)[4].(0050,0055)'", std::string("DICOM_0010_0011_\\[(\\d*)\\]_([A-Fa-f\\d]{4})_([A-Fa-f\\d]{4})_0030_0033_(004c|004C)_(004d|004D)_\\[4\\]_0050_0055"), result);
  }

  void DICOMTagPathToPersistenceKeyTemplate()
  {
    std::string result = mitk::DICOMTagPathToPersistenceKeyTemplate(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0011)'", std::string("DICOM_0010_0011"), result);
    result = mitk::DICOMTagPathToPersistenceKeyTemplate(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0011).(0020,0022).(003A,0033)'", std::string("DICOM_0010_0011_0020_0022_003A_0033"), result);
    result = mitk::DICOMTagPathToPersistenceKeyTemplate(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0011).*.(003a,003f)'", std::string("DICOM_0010_0011_$1_$2_003A_003F"), result);
    result = mitk::DICOMTagPathToPersistenceKeyTemplate(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0011).(002B,002E)[*].(0030,0033)'", std::string("DICOM_0010_0011_002B_002E_[$1]_0030_0033"), result);
    result = mitk::DICOMTagPathToPersistenceKeyTemplate(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0011).(0020,0022)[6].(003b,003e)'", std::string("DICOM_0010_0011_0020_0022_[6]_003B_003E"), result);
    result = mitk::DICOMTagPathToPersistenceKeyTemplate(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceKeyTemplate() with '(0010,0011)[*].*.(0030,0033).(004c,004d)[4].(0050,0055)'", std::string("DICOM_0010_0011_[$1]_$2_$3_0030_0033_004C_004D_[4]_0050_0055"), result);
  }

  void DICOMTagPathToPersistenceNameTemplate()
  {
    std::string result = mitk::DICOMTagPathToPersistenceNameTemplate(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0011)'", std::string("DICOM.0010.0011"), result);
    result = mitk::DICOMTagPathToPersistenceNameTemplate(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0011).(0020,0022).(003A,0033)'", std::string("DICOM.0010.0011.0020.0022.003A.0033"), result);
    result = mitk::DICOMTagPathToPersistenceNameTemplate(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0011).*.(003a,003f)'", std::string("DICOM.0010.0011.$1.$2.003A.003F"), result);
    result = mitk::DICOMTagPathToPersistenceNameTemplate(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0011).(002B,002E)[*].(0030,0033)'", std::string("DICOM.0010.0011.002B.002E.[$1].0030.0033"), result);
    result = mitk::DICOMTagPathToPersistenceNameTemplate(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0011).(0020,0022)[6].(003b,003e)'", std::string("DICOM.0010.0011.0020.0022.[6].003B.003E"), result);
    result = mitk::DICOMTagPathToPersistenceNameTemplate(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPersistenceNameTemplate() with '(0010,0011)[*].*.(0030,0033).(004c,004d)[4].(0050,0055)'", std::string("DICOM.0010.0011.[$1].$2.$3.0030.0033.004C.004D.[4].0050.0055"), result);
  }

  void DICOMTagPathToDCMTKSearchPath()
  {
    std::string result = mitk::DICOMTagPathToDCMTKSearchPath(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToDCMTKSearchPath() with '(0010,0011)'", std::string("(0010,0011)"), result);
    result = mitk::DICOMTagPathToDCMTKSearchPath(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToDCMTKSearchPath() with '(0010,0011).(0020,0022).(003A,0033)'", std::string("(0010,0011).(0020,0022).(003A,0033)"), result);
    CPPUNIT_ASSERT_THROW(mitk::DICOMTagPathToDCMTKSearchPath(deepPath_withAnyElement), mitk::Exception);
    result = mitk::DICOMTagPathToDCMTKSearchPath(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToDCMTKSearchPath() with '(0010,0011).(002B,002E)[*].(0030,0033)'", std::string("(0010,0011).(002B,002E)[*].(0030,0033)"), result);
    result = mitk::DICOMTagPathToDCMTKSearchPath(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToDCMTKSearchPath() with '(0010,0011).(0020,0022)[6].(003b,003e)'", std::string("(0010,0011).(0020,0022)[6].(003B,003E)"), result);
    CPPUNIT_ASSERT_THROW(mitk::DICOMTagPathToDCMTKSearchPath(verydeepPath), mitk::Exception);
  }

  void PropertyNameToDICOMTagPath()
  {
    mitk::DICOMTagPath result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0011");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0011)'", simplePath, result);
    result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0011.0020.0022.003A.0033");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0011).(0020,0022).(003A,0033)'", deepPath, result);
    result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0011.*.003a.003f");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0011).*.(003a,003f)'", deepPath_withAnyElement, result);
    result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0011.002B.002E.[*].0030.0033");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0011).(002B,002E)[*].(0030,0033)'", deepPath_withAnySelection, result);
    result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0011.0020.0022.[6].003b.003e");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0011).(0020,0022)[6].(003b,003e)'", deepPath_withSelection, result);
    result = mitk::PropertyNameToDICOMTagPath("DICOM.0010.0011.[*].*.0030.0033.004c.004d.[4].0050.0055");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with '(0010,0011)[*].*.(0030,0033).(004c,004d)[4].(0050,0055)'", verydeepPath, result);

    result = mitk::PropertyNameToDICOMTagPath("WRONG.0010.0011.0020.0022.0030.0033");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToDICOMTagPath() with wrong path", emptyPath, result);
  }

  void DICOMTagPathToPropertyName()
  {
    std::string result = mitk::DICOMTagPathToPropertyName(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0011)'", result, std::string("DICOM.0010.0011"));
    result = mitk::DICOMTagPathToPropertyName(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0011).(0020,0022).(003A,0033)'", result, std::string("DICOM.0010.0011.0020.0022.003A.0033"));
    result = mitk::DICOMTagPathToPropertyName(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0011).*.(003a,003f)'", result, std::string("DICOM.0010.0011.*.003A.003F"));
    result = mitk::DICOMTagPathToPropertyName(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0011).(002B,002E)[*].(0030,0033)'", result, std::string("DICOM.0010.0011.002B.002E.[*].0030.0033"));
    result = mitk::DICOMTagPathToPropertyName(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0011).(0020,0022)[6].(003b,003e)'", result, std::string("DICOM.0010.0011.0020.0022.[6].003B.003E"));
    result = mitk::DICOMTagPathToPropertyName(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing DICOMTagPathToPropertyName() with '(0010,0011)[*].*.(0030,0033).(004c,004d)[4].(0050,0055)'", result, std::string("DICOM.0010.0011.[*].*.0030.0033.004C.004D.[4].0050.0055"));
  }

  void ExecutePropertyRegEx()
  {
    std::regex regEx(mitk::DICOMTagPathToPropertyRegEx(simplePath));
    std::string result = mitk::DICOMTagPathToPropertyName(simplePath);
    CPPUNIT_ASSERT(std::regex_match(result, regEx));
    regEx = std::regex(mitk::DICOMTagPathToPropertyRegEx(deepPath));
    result = mitk::DICOMTagPathToPropertyName(deepPath);
    CPPUNIT_ASSERT(std::regex_match(result, regEx));
    regEx = std::regex(mitk::DICOMTagPathToPropertyRegEx(deepPath_withAnyElement));
    result = mitk::DICOMTagPathToPropertyName(deepPath_withAnyElement);
    auto position = result.find("*");
    if (std::string::npos != position)
    {
      result.replace(position, 1, "1234.ABCD");
      CPPUNIT_ASSERT(std::regex_match(result, regEx));
    }
    regEx = std::regex(mitk::DICOMTagPathToPropertyRegEx(deepPath_withAnySelection));
    result = mitk::DICOMTagPathToPropertyName(deepPath_withAnySelection);
    position = result.find("[*]");
    if (std::string::npos != position)
    {
      result.replace(position, 3, "[10]");
      CPPUNIT_ASSERT(std::regex_match(result, regEx));
    }
    regEx = std::regex(mitk::DICOMTagPathToPropertyRegEx(deepPath_withSelection));
    result = mitk::DICOMTagPathToPropertyName(deepPath_withSelection);
    CPPUNIT_ASSERT(std::regex_match(result, regEx));
    regEx = std::regex(mitk::DICOMTagPathToPropertyRegEx(verydeepPath));
    result = mitk::DICOMTagPathToPropertyName(verydeepPath);
    position = result.find("[*]");
    if (std::string::npos != position)
    {
      result.replace(position, 3, "[1]");
      position = result.find("*");
      if (std::string::npos != position)
      {
        result.replace(position, 1, "abcd.1234");
        CPPUNIT_ASSERT(std::regex_match(result, regEx));
      }
    }
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMTagPath)

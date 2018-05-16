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

#include "mitkDICOMProperty.h"

#include "mitkImage.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

class mitkDICOMPropertyTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMPropertyTestSuite);

  MITK_TEST(GetPropertyByDICOMTagPath);
  MITK_TEST(GetPropertyByDICOMTagPath_2);
  MITK_TEST(ConvertDICOMStrToValue);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::DICOMTagPath simplePath;
  mitk::DICOMTagPath deepPath;
  mitk::DICOMTagPath deepPath2;
  mitk::DICOMTagPath deepPath_withAnyElement;
  mitk::DICOMTagPath deepPath_withAnySelection;
  mitk::DICOMTagPath deepPath_withSelection;
  mitk::DICOMTagPath deepPath_withSelection2;

  mitk::DICOMTagPath emptyPath;

  mitk::Image::Pointer data;

  std::string simplePathStr;
  std::string deepPathStr;
  std::string deepPath2Str;
  std::string deepPath_withSelectionStr;

public:

  void setUp() override
  {
    simplePath.AddElement(0x0010, 0x0010);

    deepPath.AddElement(0x0010, 0x0011).AddElement(0x0020, 0x0022).AddElement(0x0030, 0x0033);

    deepPath2.AddElement(0x0010, 0x0011).AddElement(0x0020, 0x0023).AddElement(0x0030, 0x0033);

    deepPath_withAnyElement.AddElement(0x0010, 0x0011).AddAnyElement().AddElement(0x0030, 0x0033);

    deepPath_withAnySelection.AddElement(0x0010, 0x0011).AddAnySelection(0x0020, 0x0024).AddElement(0x0030, 0x0033);

    deepPath_withSelection.AddElement(0x0010, 0x0011).AddSelection(0x0020, 0x0024, 1).AddElement(0x0030, 0x0033);

    deepPath_withSelection2.AddElement(0x0010, 0x0011).AddSelection(0x0020, 0x0024, 2).AddElement(0x0030, 0x0033);

    simplePathStr = mitk::DICOMTagPathToPropertyName(simplePath);
    deepPathStr = mitk::DICOMTagPathToPropertyName(deepPath);
    deepPath2Str = mitk::DICOMTagPathToPropertyName(deepPath2);
    deepPath_withSelectionStr = mitk::DICOMTagPathToPropertyName(deepPath_withSelection);

    data = mitk::Image::New();
    data->GetPropertyList()->SetStringProperty(simplePathStr.c_str(), "simplePath");
    data->GetPropertyList()->SetStringProperty(deepPathStr.c_str(), "deepPath");
    data->GetPropertyList()->SetStringProperty(deepPath2Str.c_str(), "deepPath2");
    data->GetPropertyList()->SetStringProperty(deepPath_withSelectionStr.c_str(), "deepPath_withSelection");
    data->GetPropertyList()->SetStringProperty("DICOM.0003.0003", "otherPath");
    data->GetPropertyList()->SetStringProperty("not_a_dicom_prop", "not_a_dicom_prop");
  }

  void tearDown() override
  {
  }

  void GetPropertyByDICOMTagPath()
  {
    std::map< std::string, mitk::BaseProperty::Pointer> result = mitk::GetPropertyByDICOMTagPath(data, simplePath);
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_EQUAL(result.begin()->second->GetValueAsString(), std::string("simplePath"));

    result = mitk::GetPropertyByDICOMTagPath(data, deepPath);
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_EQUAL(result.begin()->second->GetValueAsString(), std::string("deepPath"));

    result = mitk::GetPropertyByDICOMTagPath(data, deepPath2);
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_EQUAL(result.begin()->second->GetValueAsString(), std::string("deepPath2"));

    result = mitk::GetPropertyByDICOMTagPath(data, deepPath_withAnyElement);
    CPPUNIT_ASSERT(result.size() == 3);
    CPPUNIT_ASSERT_EQUAL(result[deepPathStr]->GetValueAsString(), std::string("deepPath"));
    CPPUNIT_ASSERT_EQUAL(result[deepPath2Str]->GetValueAsString(), std::string("deepPath2"));
    CPPUNIT_ASSERT_EQUAL(result[deepPath_withSelectionStr]->GetValueAsString(), std::string("deepPath_withSelection"));

    result = mitk::GetPropertyByDICOMTagPath(data, deepPath_withSelection);
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_EQUAL(result[deepPath_withSelectionStr]->GetValueAsString(), std::string("deepPath_withSelection"));

    result = mitk::GetPropertyByDICOMTagPath(data, deepPath_withSelection2);
    CPPUNIT_ASSERT(result.size() == 0);

    result = mitk::GetPropertyByDICOMTagPath(data, emptyPath);
    CPPUNIT_ASSERT(result.size() == 0);
  }


  void GetPropertyByDICOMTagPath_2()
  {
    std::map< std::string, mitk::BaseProperty::Pointer> result = mitk::GetPropertyByDICOMTagPath(data->GetPropertyList(), simplePath);
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_EQUAL(result.begin()->second->GetValueAsString(), std::string("simplePath"));

    result = mitk::GetPropertyByDICOMTagPath(data->GetPropertyList(), deepPath);
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_EQUAL(result.begin()->second->GetValueAsString(), std::string("deepPath"));

    result = mitk::GetPropertyByDICOMTagPath(data->GetPropertyList(), deepPath2);
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_EQUAL(result.begin()->second->GetValueAsString(), std::string("deepPath2"));

    result = mitk::GetPropertyByDICOMTagPath(data->GetPropertyList(), deepPath_withAnyElement);
    CPPUNIT_ASSERT(result.size() == 3);
    CPPUNIT_ASSERT_EQUAL(result[deepPathStr]->GetValueAsString(), std::string("deepPath"));
    CPPUNIT_ASSERT_EQUAL(result[deepPath2Str]->GetValueAsString(), std::string("deepPath2"));
    CPPUNIT_ASSERT_EQUAL(result[deepPath_withSelectionStr]->GetValueAsString(), std::string("deepPath_withSelection"));

    result = mitk::GetPropertyByDICOMTagPath(data->GetPropertyList(), deepPath_withSelection);
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT_EQUAL(result[deepPath_withSelectionStr]->GetValueAsString(), std::string("deepPath_withSelection"));

    result = mitk::GetPropertyByDICOMTagPath(data->GetPropertyList(), deepPath_withSelection2);
    CPPUNIT_ASSERT(result.size() == 0);

    result = mitk::GetPropertyByDICOMTagPath(data->GetPropertyList(), emptyPath);
    CPPUNIT_ASSERT(result.size() == 0);
  }

  void ConvertDICOMStrToValue()
  {
    CPPUNIT_ASSERT_EQUAL(mitk::ConvertDICOMStrToValue<double>("1.35"), 1.35);
    CPPUNIT_ASSERT_EQUAL(mitk::ConvertDICOMStrToValue<double>("1"), 1.);
    CPPUNIT_ASSERT_THROW(mitk::ConvertDICOMStrToValue<int>("1.35"), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL(mitk::ConvertDICOMStrToValue<int>("1"), 1);
    CPPUNIT_ASSERT_THROW(mitk::ConvertDICOMStrToValue<double>("1,35"), mitk::Exception);
    CPPUNIT_ASSERT_THROW(mitk::ConvertDICOMStrToValue<double>("nonumber"), mitk::Exception);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMProperty)

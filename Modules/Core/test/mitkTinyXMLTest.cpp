/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
// Testing
#include "mitkTestFixture.h"
#include <mitkTestingMacros.h>

// std includes
#include <cmath>
#include <iomanip>
#include <tinyxml2.h>

// MITK includes
#include "mitkStringProperty.h"
#include <mitkNumericTypes.h>

// itksys
#include <itksys/SystemTools.hxx>

// VTK includes
#include <vtkDebugLeaks.h>

// vnl includes
#include <vnl/vnl_vector_fixed.hxx>

class mitkTinyXMLTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkTinyXMLTestSuite);

  MITK_TEST(TestingFunctionSetupWorks_Success);
  MITK_TEST(TestingReadValueFromSetupDocument_Success);
  MITK_TEST(TestingReadOutValueWorks_Success);
  MITK_TEST(TestDoubleValueWriteOut_Success);
  MITK_TEST(TestDoubleValueWriteOutManyDecimalPlaces_Success);

  CPPUNIT_TEST_SUITE_END();

private:
  const std::string m_Filename = itksys::SystemTools::GetCurrentWorkingDirectory() + "/TinyXMLTest.txt";
  const std::string m_ElementToStoreAttributeName = "DoubleTest";
  const std::string m_AttributeToStoreName = "CommaValue";

  tinyxml2::XMLDocument m_Document;
  tinyxml2::XMLElement *m_DoubleTest;

  double calcPrecision(const unsigned int requiredDecimalPlaces)
  {
    return pow(10.0, -1.0 * ((double)requiredDecimalPlaces));
  }

  bool Setup(double valueToWrite)
  {
    m_Document.Clear();
    m_DoubleTest = nullptr;

    // 1. create simple document
    m_Document.InsertEndChild(m_Document.NewDeclaration());

    auto *version = m_Document.NewElement("Version");
    version->SetAttribute("Writer", __FILE__);
    version->SetAttribute("CVSRevision", "$Revision: 17055 $");
    version->SetAttribute("FileVersion", 1);
    m_Document.InsertEndChild(version);

    // 2. store one element containing a double value with potentially many after comma digits.
    auto *vElement = m_Document.NewElement(m_ElementToStoreAttributeName.c_str());
    vElement->SetAttribute(m_AttributeToStoreName.c_str(), valueToWrite);
    m_Document.InsertEndChild(vElement);

    // 3. store in file.
    auto err = m_Document.SaveFile(m_Filename.c_str());
    return tinyxml2::XML_SUCCESS == err;
  }

public:
  void setUp() override {}

  void tearDown() override {}

  void TestingFunctionSetupWorks_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Test if Setup correctly writes data to file", Setup(1.0));
  }

  int readValueFromSetupDocument(double &readOutValue)
  {
    if (tinyxml2::XML_SUCCESS != m_Document.LoadFile(m_Filename.c_str()))
    {
      CPPUNIT_ASSERT_MESSAGE("Test Setup failed, could not open file", false);
      return tinyxml2::XML_NO_ATTRIBUTE;
    }
    else
    {
      m_DoubleTest = m_Document.FirstChildElement(m_ElementToStoreAttributeName.c_str());
      return m_DoubleTest->QueryDoubleAttribute(m_AttributeToStoreName.c_str(), &readOutValue);
    }
  }

  void TestingReadValueFromSetupDocument_Success()
  {
    if (tinyxml2::XML_SUCCESS != m_Document.LoadFile(m_Filename.c_str()))
    {
      CPPUNIT_ASSERT_MESSAGE("Test Setup failed, could not open file", tinyxml2::XML_SUCCESS != m_Document.LoadFile(m_Filename.c_str()));
    }
    else
    {
      m_DoubleTest = m_Document.FirstChildElement(m_ElementToStoreAttributeName.c_str());
      CPPUNIT_ASSERT_MESSAGE("Test Setup could open file", m_DoubleTest != nullptr);
    }
  }

  /**
   * this first test ensures we can correctly readout values from the
   * TinyXMLDocument.
   */
  void TestingReadOutValueWorks_Success()
  {
    double readValue;

    CPPUNIT_ASSERT_MESSAGE("checking if readout mechanism works.",
                           tinyxml2::XML_SUCCESS == readValueFromSetupDocument(readValue));
  }

  void TestDoubleValueWriteOut_Success()
  {
    const double valueToWrite = -1.123456;
    const int validDigitsAfterComma = 6; // indicates the number of valid digits after comma of valueToWrite
    const double neededPrecision = calcPrecision(validDigitsAfterComma + 1);
    double readValue;

    Setup(valueToWrite);
    readValueFromSetupDocument(readValue);

    CPPUNIT_ASSERT_MESSAGE("Testing if value valueToWrite  equals  readValue which was retrieved from TinyXML document",
                           mitk::Equal(valueToWrite, readValue, neededPrecision));
  }

  void TestDoubleValueWriteOutManyDecimalPlaces_Success()
  {
    const double valueToWrite = -1.12345678910111;
    const int validDigitsAfterComma = 14; // indicates the number of valid digits after comma of valueToWrite
    const double neededPrecision = calcPrecision(validDigitsAfterComma + 1);
    double readValue;

    Setup(valueToWrite);

    readValueFromSetupDocument(readValue);

    CPPUNIT_ASSERT_MESSAGE("Testing if value valueToWrite equals readValue which was retrieved from TinyXML document",
                           mitk::Equal(valueToWrite, readValue, neededPrecision));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkTinyXML)

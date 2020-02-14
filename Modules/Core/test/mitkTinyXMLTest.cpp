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
#include <tinyxml.h>

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

  TiXmlDocument m_Document;
  TiXmlElement *m_DoubleTest;

  double calcPrecision(const unsigned int requiredDecimalPlaces)
  {
    return pow(10.0, -1.0 * ((double)requiredDecimalPlaces));
  }

  bool Setup(double valueToWrite)
  {
    // 1. create simple document
    auto decl = new TiXmlDeclaration("1.0", "", ""); // TODO what to write here? encoding? etc....
    m_Document.LinkEndChild(decl);

    auto version = new TiXmlElement("Version");
    version->SetAttribute("Writer", __FILE__);
    version->SetAttribute("CVSRevision", "$Revision: 17055 $");
    version->SetAttribute("FileVersion", 1);
    m_Document.LinkEndChild(version);

    // 2. store one element containing a double value with potentially many after comma digits.
    auto vElement = new TiXmlElement(m_ElementToStoreAttributeName);
    vElement->SetDoubleAttribute(m_AttributeToStoreName, valueToWrite);
    m_Document.LinkEndChild(vElement);

    // 3. store in file.
    return m_Document.SaveFile(m_Filename);
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
    if (!m_Document.LoadFile(m_Filename))
    {
      CPPUNIT_ASSERT_MESSAGE("Test Setup failed, could not open file", false);
      return TIXML_NO_ATTRIBUTE;
    }
    else
    {
      m_DoubleTest = m_Document.FirstChildElement(m_ElementToStoreAttributeName);
      return m_DoubleTest->QueryDoubleAttribute(m_AttributeToStoreName, &readOutValue);
    }
  }

  void TestingReadValueFromSetupDocument_Success()
  {
    if (!m_Document.LoadFile(m_Filename))
    {
      CPPUNIT_ASSERT_MESSAGE("Test Setup failed, could not open file", !m_Document.LoadFile(m_Filename));
    }
    else
    {
      m_DoubleTest = m_Document.FirstChildElement(m_ElementToStoreAttributeName);
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
                           TIXML_SUCCESS == readValueFromSetupDocument(readValue));
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

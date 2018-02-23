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
// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
#include <mitkTestingMacros.h>

// std includes
#include <string>
#include <cmath>
#include <iomanip>
#include <tinyxml.h>

// MITK includes
#include <mitkNumericTypes.h>
#include "mitkStringProperty.h"

//itksys
#include <itksys/SystemTools.hxx>

// VTK includes
#include <vtkDebugLeaks.h>

//vnl includes
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
	const std::string m_filename = itksys::SystemTools::GetCurrentWorkingDirectory() + "/TinyXMLTest.txt";
	const std::string m_elementToStoreAttributeName = "DoubleTest";
	const std::string m_attributeToStoreName = "CommaValue";

	TiXmlDocument m_document;
	TiXmlElement *m_doubleTest;

	double calcPrecision(const unsigned int requiredDecimalPlaces)
	{
		return pow(10.0, -1.0 * ((double)requiredDecimalPlaces));
	}

	bool Setup(double valueToWrite)
	{
	  // 1. create simple document
	  auto decl = new TiXmlDeclaration("1.0", "", ""); // TODO what to write here? encoding? etc....
	  m_document.LinkEndChild(decl);
	
	  auto version = new TiXmlElement("Version");
	  version->SetAttribute("Writer", __FILE__);
	  version->SetAttribute("CVSRevision", "$Revision: 17055 $");
	  version->SetAttribute("FileVersion", 1);
	  m_document.LinkEndChild(version);
	
	  // 2. store one element containing a double value with potentially many after comma digits.
	  auto vElement = new TiXmlElement(m_elementToStoreAttributeName);
	  vElement->SetDoubleAttribute(m_attributeToStoreName, valueToWrite);
	  m_document.LinkEndChild(vElement);
	
	  // 3. store in file.
	  return m_document.SaveFile(m_filename);
	}

public:
	void setUp()
	{
		TiXmlDocument m_document;
	}

	void tearDown() 
	{
	}

    void TestingFunctionSetupWorks_Success()
	{
		CPPUNIT_ASSERT_MESSAGE("Test if setup and teardown correctly writes data to file", Setup(1.0));
	}

    int readValueFromSetupDocument(double &readOutValue)
	{
	
	  if (!m_document.LoadFile(m_filename))
	  {
		CPPUNIT_ASSERT_MESSAGE("Test Setup failed, could not open file", false);
	    return TIXML_NO_ATTRIBUTE;
	  }
	  else
	  {
	    m_doubleTest = m_document.FirstChildElement(m_elementToStoreAttributeName);
	    return m_doubleTest->QueryDoubleAttribute(m_attributeToStoreName, &readOutValue);
	  }
	}

	void TestingReadValueFromSetupDocument_Success()
	{  
		if (!m_document.LoadFile(m_filename))
		{
			CPPUNIT_ASSERT_MESSAGE("Test Setup failed, could not open file", !m_document.LoadFile(m_filename));
		
		}
		else
		{
			m_doubleTest = m_document.FirstChildElement(m_elementToStoreAttributeName);
			CPPUNIT_ASSERT_MESSAGE("Test Setup could open file", m_doubleTest != nullptr);
		}
	}

	/**
	* this first test ensures we can correctly readout values from the
	* TinyXMLDocument.
	*/
	void TestingReadOutValueWorks_Success()
	{	 
	   double readValue;
	 
	   CPPUNIT_ASSERT_MESSAGE("checking if readout mechanism works.", TIXML_SUCCESS == readValueFromSetupDocument(readValue));
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


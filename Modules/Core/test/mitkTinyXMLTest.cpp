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
	const std::string filename = itksys::SystemTools::GetCurrentWorkingDirectory() + "/TinyXMLTest.txt";
	const std::string elementToStoreAttributeName = "DoubleTest";
	const std::string attributeToStoreName = "CommaValue";

	double calcPrecision(const unsigned int requiredDecimalPlaces)
	{
		return pow(10.0, -1.0 * ((double)requiredDecimalPlaces));
	}

	bool Setup(double valueToWrite)
	{
	  // 1. create simple document
	  TiXmlDocument document;
	  auto decl = new TiXmlDeclaration("1.0", "", ""); // TODO what to write here? encoding? etc....
	  document.LinkEndChild(decl);
	
	  auto version = new TiXmlElement("Version");
	  version->SetAttribute("Writer", __FILE__);
	  version->SetAttribute("CVSRevision", "$Revision: 17055 $");
	  version->SetAttribute("FileVersion", 1);
	  document.LinkEndChild(version);
	
	  // 2. store one element containing a double value with potentially many after comma digits.
	  auto vElement = new TiXmlElement(elementToStoreAttributeName);
	  vElement->SetDoubleAttribute(attributeToStoreName, valueToWrite);
	  document.LinkEndChild(vElement);
	
	  // 3. store in file.
	  return document.SaveFile(filename);
	}

public:
	void setUp() {

		double valueToWrite=0;
		TiXmlDocument document;
		auto decl = new TiXmlDeclaration("1.0", "", ""); 
		auto version = new TiXmlElement("Version");
		auto vElement = new TiXmlElement(elementToStoreAttributeName);
	}

	void tearDown() {

	}

    void TestingFunctionSetupWorks_Success()
	{
		CPPUNIT_ASSERT_MESSAGE("Test if setup and teardown correctly writes data to file", Setup(1.0));
	}

    int readValueFromSetupDocument(double &readOutValue)
	{
	  TiXmlDocument document;
	
	  if (!document.LoadFile(filename))
	  {
		CPPUNIT_ASSERT_MESSAGE("Test Setup failed, could not open file", false);
	    return TIXML_NO_ATTRIBUTE;
	  }
	  else
	  {
	    TiXmlElement *doubleTest = document.FirstChildElement(elementToStoreAttributeName);
	    return doubleTest->QueryDoubleAttribute(attributeToStoreName, &readOutValue);
	  }
	}

	void TestingReadValueFromSetupDocument_Success()
	{  
		TiXmlDocument document;

		if (!document.LoadFile(filename))
		{
			CPPUNIT_ASSERT_MESSAGE("Test Setup failed, could not open file", !document.LoadFile(filename));
		
		}
		else
		{
			TiXmlElement *doubleTest = document.FirstChildElement(elementToStoreAttributeName);
			CPPUNIT_ASSERT_MESSAGE("Test Setup could open file", doubleTest != nullptr);
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


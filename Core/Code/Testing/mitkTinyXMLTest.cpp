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

#include <mitkNumericTypes.h>

#include <tinyxml.h>
#include <string>
#include <cmath>
#include <iomanip>

#include <itksys/SystemTools.hxx>


static const std::string  filename = itksys::SystemTools::GetCurrentWorkingDirectory() + "/TinyXMLTest.txt";
static const std::string  elementToStoreAttributeName = "DoubleTest";
static const std::string  attributeToStoreName        = "CommaValue";

static double calcPrecision(const unsigned int requiredDecimalPlaces)
{
  return pow(10.0, -1.0 * ((double) requiredDecimalPlaces));
}

/**
 * create a simple xml document which stores the values
 * @param valueToWrite  value which should be stored
 * @return  true, if document was successfully created.
 */
static bool Setup(double valueToWrite)
{
  // 1. create simple document
  TiXmlDocument document;
  TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" ); // TODO what to write here? encoding? etc....
  document.LinkEndChild( decl );

  TiXmlElement* version = new TiXmlElement("Version");
  version->SetAttribute("Writer",  __FILE__ );
  version->SetAttribute("CVSRevision",  "$Revision: 17055 $" );
  version->SetAttribute("FileVersion",  1 );
  document.LinkEndChild(version);

  // 2. store one element containing a double value with potentially many after comma digits.
  TiXmlElement* vElement = new TiXmlElement( elementToStoreAttributeName );
  vElement->SetDoubleAttribute( attributeToStoreName, valueToWrite );
  document.LinkEndChild(vElement);

  // 3. store in file.
  return document.SaveFile( filename );
}

static int readValueFromSetupDocument(double& readOutValue)
{
  TiXmlDocument document;

  if (!document.LoadFile(filename))
  {
    MITK_TEST_CONDITION_REQUIRED(false, "Test Setup failed, could not open " << filename);
    return TIXML_NO_ATTRIBUTE;
  }
  else
  {
    TiXmlElement* doubleTest = document.FirstChildElement(elementToStoreAttributeName);
    return doubleTest->QueryDoubleAttribute(attributeToStoreName, &readOutValue);
  }
}

/**
 *
 * @return true if TearDown was successful.
 */
static bool TearDown()
{
  return !remove(filename.c_str());
}

static void Test_Setup_works()
{
  MITK_TEST_CONDITION_REQUIRED(Setup(1.0) && TearDown(),
      "Test if setup and teardown correctly writes data to " << filename << " and deletes the file after the test");
}

/**
 * this first test ensures we can correctly readout values from the
 * TinyXMLDocument.
 */
static void Test_ReadOutValue_works()
{
  Setup(1.0);

  double readValue;

  MITK_TEST_CONDITION_REQUIRED(TIXML_SUCCESS == readValueFromSetupDocument(readValue),
      "checking if readout mechanism works.");
}



static void Test_DoubleValueWriteOut()
{
  const double valueToWrite          = -1.123456;
  const int    validDigitsAfterComma = 6; // indicates the number of valid digits after comma of valueToWrite
  const double neededPrecision       = calcPrecision(validDigitsAfterComma + 1);
  double       readValue;

  Setup(valueToWrite);

  readValueFromSetupDocument(readValue);

  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(valueToWrite, readValue, neededPrecision),
      std::setprecision(validDigitsAfterComma) <<
      "Testing if value " << valueToWrite << " equals " << readValue
      << " which was retrieved from TinyXML document");

  TearDown();
}

static void Test_DoubleValueWriteOut_manyDecimalPlaces()
{
  const double valueToWrite          = -1.12345678910111;
  const int    validDigitsAfterComma = 14; // indicates the number of valid digits after comma of valueToWrite
  const double neededPrecision       = calcPrecision(validDigitsAfterComma + 1);
  double       readValue;

  Setup(valueToWrite);

  readValueFromSetupDocument(readValue);

  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(valueToWrite, readValue, neededPrecision),
      std::setprecision(validDigitsAfterComma) <<
      "Testing if value " << valueToWrite << " equals " << readValue
      << " which was retrieved from TinyXML document");

  TearDown();
}



int mitkTinyXMLTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("TinyXMLTest");

  Test_Setup_works();
  Test_ReadOutValue_works();
  Test_DoubleValueWriteOut();
  Test_DoubleValueWriteOut_manyDecimalPlaces();

  MITK_TEST_END()
}

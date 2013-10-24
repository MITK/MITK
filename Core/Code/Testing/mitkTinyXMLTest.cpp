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

#include <tinyxml.h>
#include <string.h>
#include <itksys/SystemTools.hxx>


static const std::string filename = itksys::SystemTools::GetCurrentWorkingDirectory() + "/TinyXMLTest.txt";
static const std::string elementToStoreAttributeName = "DoubleTest";
static const std::string attributeToStoreName        = "CommaValue";

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

static void TearDown()
{
  // TODO delete created file, ...
}

static void Test_Setup_works()
{
  MITK_TEST_CONDITION_REQUIRED(Setup(1.0), "Test Setup failed, could not write xml to " << filename);
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
  const int    validDigitsAfterComma = 7; // indicates the number of valid digits after comma of valueToWrite
  const double neededPrecision       = 1.0 / ((double) validDigitsAfterComma);

  Setup(valueToWrite);

  double readValue;

  readValueFromSetupDocument(readValue);

  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(valueToWrite, neededPrecision),
      std::setprecision(validDigitsAfterComma) << "Testing if value " << valueToWrite << " equals " << readValue
      << " which was retrieved from TinyXML document");

  TearDown();
}

int mitkTinyXMLTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("TinyXMLTest");

  Test_Setup_works();
  Test_ReadOutValue_works();
  Test_DoubleValueWriteOut();

  MITK_TEST_END()
}

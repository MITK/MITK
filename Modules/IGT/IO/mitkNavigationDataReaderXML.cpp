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

#include "mitkNavigationDataReaderXML.h"
#include <itksys/SystemTools.hxx>
#include <fstream>
#include "tinyxml.h"

//includes for exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"

mitk::NavigationDataReaderXML::NavigationDataReaderXML()
  : m_parentElement(0), m_currentNode(0)
{
}

mitk::NavigationDataReaderXML::~NavigationDataReaderXML()
{

}

mitk::NavigationDataSet::Pointer mitk::NavigationDataReaderXML::Read(std::string fileName)
{
  //save old locale
  char * oldLocale;
  oldLocale = setlocale( LC_ALL, 0 );

  //define own locale
  std::locale C("C");
  setlocale( LC_ALL, "C" );

  m_FileName = fileName;

  TiXmlDocument document;
  if ( !document.LoadFile(fileName))
  {
    mitkThrowException(mitk::IGTIOException) << "File '"<<fileName<<"' could not be loaded.";
  }

  TiXmlElement* m_DataElem = document.FirstChildElement("Version");
  if(!m_DataElem)
  {
    // for backwards compatibility of version tag
    m_DataElem = document.FirstChildElement("Data");
    if(!m_DataElem)
    {
      mitkThrowException(mitk::IGTIOException) << "Data element not found.";
    }

  }

  if (m_DataElem->QueryIntAttribute("Ver", &m_FileVersion) != TIXML_SUCCESS)
  {
    if (m_DataElem->QueryIntAttribute("version", &m_FileVersion) != TIXML_SUCCESS)
    {
      mitkThrowException(mitk::IGTIOException) << "Version not specified in XML file.";
    }
  }

  if (m_FileVersion != 1)
  {
    mitkThrowException(mitk::IGTIOException) << "File format version "<<m_FileVersion<<" is not supported.";
  }

  m_parentElement = document.FirstChildElement("Data");
  if(!m_parentElement)
  {
    mitkThrowException(mitk::IGTIOException) << "Data element not found.";
  }

  m_parentElement->QueryIntAttribute("ToolCount", &m_NumberOfOutputs);

  mitk::NavigationDataSet::Pointer navigationDataSet = this->ReadNavigationDataSet();

  //switch back to old locale
  setlocale( LC_ALL, oldLocale );

  return navigationDataSet;
}

mitk::NavigationDataSet::Pointer mitk::NavigationDataReaderXML::Read(std::istream* stream)
{
  //save old locale
  char * oldLocale;
  oldLocale = setlocale( LC_ALL, 0 );

  //define own locale
  std::locale C("C");
  setlocale( LC_ALL, "C" );

  // first get the file version
  m_FileVersion = this->GetFileVersion(stream);

  // check if we have a valid version: m_FileVersion has to be always bigger than 1 for playing
  if (m_FileVersion < 1)
  {
    StreamInvalid("Playing not possible. Invalid file version!");
    return 0;
  }

  m_NumberOfOutputs = this->GetNumberOfNavigationDatas(stream);
  if (m_NumberOfOutputs == 0) { return 0; }

  mitk::NavigationDataSet::Pointer dataSet = this->ReadNavigationDataSet();

  //switch back to old locale
  setlocale( LC_ALL, oldLocale );

  return dataSet;
}

mitk::NavigationDataSet::Pointer mitk::NavigationDataReaderXML::ReadNavigationDataSet()
{
  mitk::NavigationDataSet::Pointer navigationDataSet = mitk::NavigationDataSet::New(m_NumberOfOutputs);
  mitk::NavigationData::Pointer curNavigationData;

  do
    {
      std::vector<mitk::NavigationData::Pointer> navDatas(m_NumberOfOutputs);
      for (unsigned int n = 0; n < m_NumberOfOutputs; ++n)
      {
        curNavigationData = this->ReadVersion1();

        if (curNavigationData.IsNull())
        {
          if (n != 0)
          {
            MITK_WARN("mitkNavigationDataReaderXML")
                << "Different number of NavigationData objects for different tools. Ignoring last ones.";
          }
          break;
        }
        navDatas.at(n) = curNavigationData;
      }

      if (curNavigationData.IsNotNull())
      {
        navigationDataSet->AddNavigationDatas(navDatas);
      }
    }
    while (curNavigationData.IsNotNull());

    return navigationDataSet;
}

mitk::NavigationData::Pointer mitk::NavigationDataReaderXML::ReadVersion1()
{
  if ( !m_parentElement )
  {
    mitkThrowException(mitk::IGTIOException)
        << "Reading XML is not possible. Parent element is not set.";
  }

  TiXmlElement* elem;
  m_currentNode = m_parentElement->IterateChildren(m_currentNode);

  bool delElem;

  if(m_currentNode)
  {
    elem = m_currentNode->ToElement();
    if(elem==NULL)
    {
      mitkThrowException(mitk::IGTException) << "Cannot find element: Is this file damaged?";
    }
    delElem = false;
  }

  else
  {
    elem = new TiXmlElement("");
    delElem = true;
  }


  mitk::NavigationData::Pointer nd = this->ReadNavigationData(elem);

  if(delElem) { delete elem; }

  return nd;
}

mitk::NavigationData::Pointer mitk::NavigationDataReaderXML::ReadNavigationData(TiXmlElement* elem)
{
  if (elem == NULL) {mitkThrow() << "Error: Element is NULL!";}

  mitk::NavigationData::Pointer nd = mitk::NavigationData::New();

  mitk::NavigationData::PositionType position;
  mitk::NavigationData::OrientationType orientation(0.0,0.0,0.0,0.0);
  mitk::NavigationData::TimeStampType timestamp = -1;
  mitk::NavigationData::CovarianceMatrixType matrix;

  bool hasPosition = true;
  bool hasOrientation = true;
  bool dataValid = false;

  position.Fill(0.0);
  matrix.SetIdentity();

  elem->QueryDoubleAttribute("Time",&timestamp);
  if (timestamp == -1)
  {
    return NULL;  //the calling method should check the return value if it is valid/not NULL
  }

  elem->QueryDoubleAttribute("X", &position[0]);
  elem->QueryDoubleAttribute("Y", &position[1]);
  elem->QueryDoubleAttribute("Z", &position[2]);

  elem->QueryDoubleAttribute("QX", &orientation[0]);
  elem->QueryDoubleAttribute("QY", &orientation[1]);
  elem->QueryDoubleAttribute("QZ", &orientation[2]);
  elem->QueryDoubleAttribute("QR", &orientation[3]);

  elem->QueryDoubleAttribute("C00", &matrix[0][0]);
  elem->QueryDoubleAttribute("C01", &matrix[0][1]);
  elem->QueryDoubleAttribute("C02", &matrix[0][2]);
  elem->QueryDoubleAttribute("C03", &matrix[0][3]);
  elem->QueryDoubleAttribute("C04", &matrix[0][4]);
  elem->QueryDoubleAttribute("C05", &matrix[0][5]);
  elem->QueryDoubleAttribute("C10", &matrix[1][0]);
  elem->QueryDoubleAttribute("C11", &matrix[1][1]);
  elem->QueryDoubleAttribute("C12", &matrix[1][2]);
  elem->QueryDoubleAttribute("C13", &matrix[1][3]);
  elem->QueryDoubleAttribute("C14", &matrix[1][4]);
  elem->QueryDoubleAttribute("C15", &matrix[1][5]);

  int tmpval = 0;
  elem->QueryIntAttribute("Valid", &tmpval);
  if (tmpval == 0)
    dataValid = false;
  else
    dataValid = true;

  tmpval = 0;
  elem->QueryIntAttribute("hO", &tmpval);
  if (tmpval == 0)
    hasOrientation = false;
  else
    hasOrientation = true;

  tmpval = 0;
  elem->QueryIntAttribute("hP", &tmpval);
  if (tmpval == 0)
    hasPosition = false;
  else
    hasPosition = true;

  nd->SetIGTTimeStamp(timestamp);
  nd->SetPosition(position);
  nd->SetOrientation(orientation);
  nd->SetCovErrorMatrix(matrix);
  nd->SetDataValid(dataValid);
  nd->SetHasOrientation(hasOrientation);
  nd->SetHasPosition(hasPosition);


  return nd;
}

// -- deprecated | begin
unsigned int mitk::NavigationDataReaderXML::GetFileVersion(std::istream* stream)
{
  if (stream==NULL)
  {
    MITK_ERROR << "No input stream set!";
    mitkThrowException(mitk::IGTIOException)<<"No input stream set!";
  }
  if (!stream->good())
  {
    MITK_ERROR << "Stream is not good!";
    mitkThrowException(mitk::IGTIOException)<<"Stream is not good!";
  }
  int version = 1;

  TiXmlDeclaration* dec = new TiXmlDeclaration();
  *stream >> *dec;
  if(strcmp(dec->Version(),"") == 0)
  {
    MITK_ERROR << "The input stream seems to have XML incompatible format";
    mitkThrowException(mitk::IGTIOException) << "The input stream seems to have XML incompatible format";
  }

  m_parentElement = new TiXmlElement("");
  *stream >> *m_parentElement; //2nd line this is the file version

  std::string tempValue = m_parentElement->Value();
  if(tempValue != "Version")
  {
      if(tempValue == "Data"){
          m_parentElement->QueryIntAttribute("version",&version);
      }
  }
  else
  {
    m_parentElement->QueryIntAttribute("Ver",&version);
  }

  if (version > 0) { return version; }
  else { return 0; }
}

unsigned int mitk::NavigationDataReaderXML::GetNumberOfNavigationDatas(std::istream* stream)
{
  if (stream == NULL)
  {
    MITK_ERROR << "No input stream set!";
    mitkThrowException(mitk::IGTException)<<"No input stream set!";
  }
  if (!stream->good())
  {
    MITK_ERROR << "Stream not good!";
    mitkThrowException(mitk::IGTException)<<"Stream not good!";
  }

  //If something has changed in a future version of the XML definition e.g. navigationcount or addional parameters
  //catch this here with a select case block (see GenerateData() method)

  int numberOfTools = 0;

  std::string tempValue = m_parentElement->Value();
  if(tempValue == "Version"){
    *stream >> *m_parentElement;
  }
  m_parentElement->QueryIntAttribute("ToolCount",&numberOfTools);

  if (numberOfTools > 0) { return numberOfTools; }

  return 0;
}

void mitk::NavigationDataReaderXML::StreamInvalid(std::string message)
{
  m_StreamEnd = true;
  m_ErrorMessage = message;
  m_StreamValid = false;
  mitkThrowException(mitk::IGTIOException) << "Invalid stream!";
}
// -- deprecated | end


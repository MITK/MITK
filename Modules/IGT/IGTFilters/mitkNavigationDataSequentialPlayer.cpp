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

#include "mitkNavigationDataSequentialPlayer.h"

//for the pause
#include <itksys/SystemTools.hxx>

#include <mitkTimeStamp.h>
#include <fstream>
#include <sstream>

//Exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"

mitk::NavigationDataSequentialPlayer::NavigationDataSequentialPlayer() 
  : mitk::NavigationDataPlayerBase()
  , m_Doc(new TiXmlDocument)
  , m_DataElem(0)
  , m_CurrentElem(0)
  , m_Repeat(false)
  , m_NumberOfSnapshots(0)
  , m_LastGoTo(0)
{
}


mitk::NavigationDataSequentialPlayer::~NavigationDataSequentialPlayer()
{
  delete m_Doc;
}

void mitk::NavigationDataSequentialPlayer::ReinitXML()
{
  m_DataElem = m_Doc->FirstChildElement("Data");
  int toolcount;
  if(!m_DataElem)
  {
    //throwing an exception
    mitkThrowException(mitk::IGTException) << "Data element not found";
    MITK_WARN << "Data element not found";
  }
  else
  {
    m_DataElem->QueryIntAttribute("ToolCount", &toolcount);
    this->SetNumberOfOutputs(toolcount);

    mitk::NavigationData::Pointer emptyNd = mitk::NavigationData::New();
    mitk::NavigationData::PositionType position;
    mitk::NavigationData::OrientationType orientation(0.0,0.0,0.0,0.0);
    position.Fill(0.0);

    emptyNd->SetPosition(position);
    emptyNd->SetOrientation(orientation);
    emptyNd->SetDataValid(false);

    mitk::NavigationData::Pointer tmp;
    for (unsigned int index = 0; index < this->GetNumberOfOutputs(); index++)
    {
      tmp = mitk::NavigationData::New();
      tmp->Graft(emptyNd);
      this->SetNthOutput(index, tmp);
    }

    // find out _NumberOfSnapshots
    m_NumberOfSnapshots = 0;
    TiXmlElement* nextND = m_DataElem->FirstChildElement("NavigationData");
    while(nextND)
    {
      ++m_NumberOfSnapshots;
      nextND = nextND->NextSiblingElement("NavigationData");
    }
    // e.g. 12 nd found and 2 tools used => number of snapshots is 12:2=6
    m_NumberOfSnapshots = m_NumberOfSnapshots/toolcount;

  }
}

void mitk::NavigationDataSequentialPlayer::GoToSnapshot(int i)
{
  assert(m_DataElem);

  int numOfUpdateCalls = 0;

  // i.e. number of snapshots 10
  // goto(7), m_LastGoTo=3 => numOfUpdateCalls = 4
  if(m_LastGoTo <= i)
    numOfUpdateCalls = i - m_LastGoTo;
  // goto(4), m_LastGoTo=7 => numOfUpdateCalls = 7
  else
  {
    if(!m_Repeat)
    {
      //throwing an exception
      mitkThrowException(mitk::IGTException)<<"cannot go back to snapshot " << i << " because the "
          << this->GetNameOfClass() << " is configured to not repeat the"
          << " navigation data";

      MITK_WARN << "cannot go back to snapshot " << i << " because the "
          << this->GetNameOfClass() << " is configured to not repeat the"
          << " navigation data";

    }
    else
    {
      numOfUpdateCalls = (m_NumberOfSnapshots - m_LastGoTo) + i;
    }
  }

  for(int j=0; j<numOfUpdateCalls; ++j)
    this->Update();

  m_LastGoTo = i;
}

void mitk::NavigationDataSequentialPlayer::
    SetFileName(const std::string& _FileName)
{
  m_FileName = _FileName;
  // if Loading wasnt succesfull
  // (!m_Doc->LoadFile(m_FileName)=false) wherase (m_Doc->LoadFile(m_FileName)=true)

  if(!m_Doc->LoadFile(m_FileName))
  {
    this->SetNumberOfOutputs(0);
    std::ostringstream s;
    s << "File " << _FileName << " could not be loaded";
    //may be we need to change to mitk::exception
    mitkThrowException(mitk::IGTIOException)<<s.str();
    //throw std::invalid_argument(s.str());
  }
  else
    this->ReinitXML();

  this->Modified();
}

//NEW PART is ADDED
void mitk::NavigationDataSequentialPlayer::
    SetXMLString(const std::string& _XMLString)
{
  m_XMLString = _XMLString;
  if((m_Doc->Parse( m_XMLString.c_str()))== NULL)
  {
  this->ReinitXML();
  } else 
  {
    //if the string is not an XML string
   std::ostringstream s;
   s << "String" << _XMLString << " is not an XML string";
   mitkThrowException(mitk::IGTIOException)<<s.str();
  } 

  this->Modified();
}

void mitk::NavigationDataSequentialPlayer::GenerateData()
{
  assert(m_DataElem);
  // very important: go through the tools (there could be more than one)
  mitk::NavigationData::Pointer tmp;
  //MITK_INFO << "this->GetNumberOfOutputs()" << this->GetNumberOfOutputs();
  for (unsigned int index = 0; index < this->GetNumberOfOutputs(); index++)
  {
    //MITK_INFO << "index" << index;
    // go to the first element
    if(!m_CurrentElem)
      m_CurrentElem = m_DataElem->FirstChildElement("NavigationData");
    // go to the next element
    else
      m_CurrentElem = m_CurrentElem->NextSiblingElement();

    // if repeat is on: go back to the first element (prior calls delivered NULL
    // elem)
    if(!m_CurrentElem && m_Repeat)
      m_CurrentElem = m_DataElem->FirstChildElement("NavigationData");

    mitk::NavigationData* output = this->GetOutput(index);
    tmp = this->ReadVersion1();
    if(tmp.IsNotNull())
      {
      output->Graft(tmp);
      m_StreamValid = true;
      }
    else // no valid output
      {
      output->SetDataValid(false);
      m_StreamValid = false;
      //throwing exception 
      mitkThrowException(mitk::IGTException)<<"Error: Cannot parse input file.";
      //m_ErrorMessage = "Error: Cannot parse input file.";
      }
  }
}

mitk::NavigationData::Pointer mitk::NavigationDataSequentialPlayer::ReadVersion1()
{

  TiXmlElement* elem = m_CurrentElem;

  if(!elem)
    return NULL;

  return this->ReadNavigationData(elem);
}

void mitk::NavigationDataSequentialPlayer::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}




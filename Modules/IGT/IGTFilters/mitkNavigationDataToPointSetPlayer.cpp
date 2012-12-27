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

#include "mitkNavigationDataToPointSetPlayer.h"

//for the pause
#include <itksys/SystemTools.hxx>

#include <mitkTimeStamp.h>
#include <fstream>
#include <sstream>

#include <math.h>

mitk::NavigationDataToPointSetPlayer::NavigationDataToPointSetPlayer()
  : m_Doc(new TiXmlDocument)
  , m_DataElem(0)
  , m_CurrentElem(0)
  , m_Repeat(false)
  , m_NumberOfSnapshots(0)
  , m_LastGoTo(0)
{
}


mitk::NavigationDataToPointSetPlayer::~NavigationDataToPointSetPlayer()
{
  delete m_Doc;
}

void mitk::NavigationDataToPointSetPlayer::ReinitXML()
{
  m_NDPointSet.clear();
  m_PointSetFilter = mitk::NavigationDataToPointSetFilter::New();

  m_DataElem = m_Doc->FirstChildElement("Data");
  int toolcount;
  if(!m_DataElem)
    MITK_WARN << "Data element not found";
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

    NavigationData::Pointer nd;

    m_NumberOfSnapshots = 0;
    TiXmlElement* nextND = m_DataElem->FirstChildElement("NavigationData");

    while(nextND)
    {
      ++m_NumberOfSnapshots;
      nextND = nextND->NextSiblingElement("NavigationData");
    }

    // e.g. 12 nd found and 2 tools used => number of snapshots is 12:2=6
    m_NumberOfSnapshots = m_NumberOfSnapshots/toolcount;

    /*NavigationData::TimeStampType recordedTime = (lastTimestamp-firstTimestamp) / 1000;
    int frameRate = static_cast<int>(floor(1000 / (float) (m_NumberOfSnapshots/recordedTime) + 0.5));*/

  }
}



void mitk::NavigationDataToPointSetPlayer::
    SetFileName(const std::string& _FileName)
{
  m_FileName = _FileName;

  if(!m_Doc->LoadFile(m_FileName))
  {
    this->SetNumberOfOutputs(0);
    std::ostringstream s;
    s << "File " << _FileName << " could not be loaded";
    throw std::invalid_argument(s.str());
  }
  else
  {
    this->ReinitXML();
  }

  this->Modified();
}


void mitk::NavigationDataToPointSetPlayer::GenerateData()
{
  assert(m_DataElem);

  // very important: go through the tools (there could be more then one)
  mitk::NavigationData::Pointer tmp;
  for (unsigned int index = 0; index < this->GetNumberOfOutputs(); index++)
  {
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
      output->Graft(tmp);
    else // no valid output
      output->SetDataValid(false);
  }
}


void mitk::NavigationDataToPointSetPlayer::StartPlaying()
{
  //TODO
}

void mitk::NavigationDataToPointSetPlayer::StopPlaying()
{
  //TODO
}

void mitk::NavigationDataToPointSetPlayer::Pause()
{
  //TODO
}

void mitk::NavigationDataToPointSetPlayer::Resume()
{
  //TODO
}


//TODO
const bool mitk::NavigationDataToPointSetPlayer::IsAtEnd()
{
  bool result = false;
  return result;
}

mitk::NavigationData::Pointer mitk::NavigationDataToPointSetPlayer::ReadVersion1()
{
  TiXmlElement* elem = m_CurrentElem;

  if(!elem)
    return NULL;

  return this->ReadNavigationData(elem);
}

void mitk::NavigationDataToPointSetPlayer::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}

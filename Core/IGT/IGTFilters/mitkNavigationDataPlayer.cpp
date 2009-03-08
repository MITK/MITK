/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-02-10 18:08:54 +0100 (Di, 10 Feb 2009) $
Version:   $Revision: 16228 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataPlayer.h"

//for the pause 
#include <itksys/SystemTools.hxx>

#include <mitkTimeStamp.h>

mitk::NavigationDataPlayer::NavigationDataPlayer() 
{
  m_PlayerMode = NormalFile;
  this->m_MultiThreader = itk::MultiThreader::New();
  m_NumberOfOutputs = 0;
  m_Pause = false;
  m_Playing = false;
  //To get a start time
  mitk::TimeStamp::GetInstance()->StartTracking(this);

}


mitk::NavigationDataPlayer::~NavigationDataPlayer()
{

}


void mitk::NavigationDataPlayer::GenerateData()
{
  if (!m_Playing)
  {
    return;
  }

  //first of all get current time
  TimeStampType now = mitk::TimeStamp::GetInstance()->GetElapsed();

  //now we make a little time arithmetic
  TimeStampType timeSinceStart = now - m_StartPlayingTimeStamp;
  
  //init the vectors
  std::vector<NavigationData::Pointer> nextCandidates; 
  std::vector<NavigationData::Pointer> lastCandidates; 

  for (unsigned int index=0; index < m_NumberOfOutputs; index++)
  {
    nextCandidates.push_back(m_NextToPlayNavigationData[index]);
    lastCandidates.push_back(m_NextToPlayNavigationData[index]);
    
    m_CurrentTimeOfData[index] = timeSinceStart + m_StartTimeOfData[index];

  }

  //now try to find next NavigationData in the stream
  //the timestamps of each tracked NavigationData is equal
  while(nextCandidates[0]->GetTimeStamp() < m_CurrentTimeOfData[0])
  {

    for (unsigned int index=0; index < m_NumberOfOutputs; index++)
    {
      lastCandidates[index] = nextCandidates[index];

      switch(m_FileVersion)
      {
      case 1: 
        nextCandidates[index] = ReadVersion1();
        break;
      default: //this case should not happen! therefore the return at this point
        return;
        break;
      }

      //check if the input stream delivered a correct NavigationData object
      for (unsigned int i=0; i < m_NumberOfOutputs; i++)
      {
        if (nextCandidates[i].IsNull())
        {
          StopPlaying();
          return;
        }
      }
    }

  }
  //Now lastCandidates holds the new output and nextCandidates is stored to the m_NextToPlay vector
  for (unsigned int index=0; index < m_NumberOfOutputs; index++)
  {
    std::cout << lastCandidates[index]->GetPosition() << std::endl;
    mitk::NavigationData* output = this->GetOutput(index);
    
    output->SetPosition(lastCandidates[index]->GetPosition());
    output->SetOrientation(lastCandidates[index]->GetOrientation());
    output->SetTimeStamp(lastCandidates[index]->GetTimeStamp());

    m_NextToPlayNavigationData[index] = nextCandidates[index];
  }

}


void mitk::NavigationDataPlayer::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}


void mitk::NavigationDataPlayer::StartPlaying()
{
  std::stringstream ss;
  std::istream* stream=NULL;

  //TODO Save date to filename
  ss << m_FilePath << "/" << m_FileName;
  switch(m_PlayerMode)
  {
    case NormalFile:

      //Check if there is a file name and path
      if (m_FileName == "" || m_FilePath == "")
      {
        stream = NULL;
        std::cout << "No file name or file path set the output is redirected to the console";
      }
      else
      {
        stream = new std::ifstream(ss.str().c_str());
      }
      break;
    case ZipFile:
      stream = NULL;
      std::cout << "Sorry no ZipFile support yet";
      break;
    default:
      stream = NULL;
      break;
  }
  StartPlaying(stream);
}

void mitk::NavigationDataPlayer::StartPlaying( std::istream* stream )
{
  if (stream == NULL)
  {
    m_Playing = false;
    std::cout << "Playing not possible. Wrong file name or path? " << std::endl;
  }
  m_Stream = stream;
  
  m_Playing = true; //starts the player
  
  GetFileVersion();

  GetNumberOfTrackedTools();

  if (m_NumberOfOutputs > 0)
  {
    //Generate the output
    for(unsigned int index=0; index < m_NumberOfOutputs; index++)
    {
      DataObjectPointer newOutput = this->MakeOutput(index);
      this->SetNthOutput(index, newOutput);     
    }

    GetFirstData();
    m_StartPlayingTimeStamp = mitk::TimeStamp::GetInstance()->GetElapsed();
  }
  else
  {
    StopPlaying();
  }

  //TODO close thread when this class is deconstructed?
  //m_ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartPlaying, this);

}

void mitk::NavigationDataPlayer::GetFileVersion()
{
  if (m_Stream==NULL)
  {
    std::cout << "No input stream set!" << std::endl;
    return;
  }
  int version;
  char str[255];
  m_Stream->getline(str,255); //1st line this is the xml header

  TiXmlElement* elem = new TiXmlElement("");
  *m_Stream >> *elem; //2nd line this is the file version


  elem->QueryIntAttribute("Ver",&version);
  
  if (version > 0)
    m_FileVersion = version;
  else
    m_FileVersion = 1;

}

void mitk::NavigationDataPlayer::GetNumberOfTrackedTools()
{
  if (m_Stream==NULL)
  {
    std::cout << "No input stream set!" << std::endl;
    return;
  }
  
  //If something has changed in a future version of the XML definition e.g. toolcount or addional parameters 
  //catch this here with a select case block (see play() method)

  int numberOfTools = 0;
  
  //if you stream to a TiXmlElement the whole Tag with its entire data is streamed to the element
  //therefore we only read one line and parse this
  char str[255];
  
  m_Stream->getline(str,255); //read the line until end
  m_Stream->getline(str,255); //3th line this one indicates additional parameters of the NavigationData object
  
  std::stringstream ss;
  ss << str;

  std::cout << ss;
  TiXmlElement* elem = new TiXmlElement("");
  ss >> *elem; 
  //*str >> *elem; 


  elem->QueryIntAttribute("ToolCount",&numberOfTools);

  if (numberOfTools > 0)
    m_NumberOfOutputs = numberOfTools;
  else
    m_NumberOfOutputs = 0;

  //init the vector
  for (unsigned int x=0; x < m_NumberOfOutputs; x++)
  {
      m_NextToPlayNavigationData.push_back(NULL);
      m_StartTimeOfData.push_back(0.0);
      m_CurrentTimeOfData.push_back(0.0);
  }
}
void mitk::NavigationDataPlayer::PlayInThread()
{    
  unsigned int delay=0;
  unsigned int nextOutputNumber=0;

  mitk::NavigationData::Pointer lastNavigationData = NULL;
  mitk::NavigationData::Pointer nextNavigationData = NULL;  

  try
  {
    while (m_Playing)
    {
      if(nextNavigationData.IsNotNull())
      this->SetNthOutput(nextOutputNumber, nextNavigationData);
      
      lastNavigationData = nextNavigationData;
      nextNavigationData = NavigationData::New();

      //now read data into nextNavigationData with the right version number

      switch(m_FileVersion)
      {
      case 1: 
        nextNavigationData = ReadVersion1();
        break;
      default: //this case should not happen! therefore the return at this point
        return;
        break;
      }
      
      //calculate the delay only if this is not the first run of this loop
      if(nextNavigationData.IsNotNull())
        delay = nextNavigationData->GetTimeStamp() - lastNavigationData->GetTimeStamp();
      else
        delay = 0;

      itksys::SystemTools::Delay(delay);
    }
  }
  catch(...)
  {
    std::cout << "Playing stoped due to a failure with the input stream" << std::endl;
    this->StopPlaying();
  }
}
ITK_THREAD_RETURN_TYPE mitk::NavigationDataPlayer::ThreadStartPlaying(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  if (pInfo->UserData == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  NavigationDataPlayer *player = (NavigationDataPlayer*)pInfo->UserData;

  if (player != NULL) 
    player->PlayInThread();

  return ITK_THREAD_RETURN_VALUE;
}

mitk::NavigationData::Pointer mitk::NavigationDataPlayer::ReadVersion1()
{
  mitk::NavigationData::Pointer nd = mitk::NavigationData::New();
  mitk::NavigationData::PositionType position;
  mitk::NavigationData::OrientationType orientation;
  mitk::NavigationData::TimeStampType timestamp = -1; 
  
  TiXmlElement* elem = new TiXmlElement("");
  *m_Stream >> *elem;

  //TODO check here if EOF (the query don't change the timestamp value which should always be > 0)
  elem->QueryDoubleAttribute("Time",&timestamp);
  if (timestamp == -1)
  {
    return NULL;
  }
  timestamp = mitk::TimeStamp::GetInstance()->GetElapsed();
  elem->QueryFloatAttribute("X",&position[0]);
  elem->QueryFloatAttribute("Y",&position[1]);
  elem->QueryFloatAttribute("Z",&position[2]);

  elem->QueryFloatAttribute("QX",&orientation[0]);
  elem->QueryFloatAttribute("QY",&orientation[1]);
  elem->QueryFloatAttribute("QZ",&orientation[2]);
  elem->QueryFloatAttribute("QR",&orientation[3]);
  
  std::cout << position << orientation << std::endl;
  nd->SetTimeStamp(timestamp);
  nd->SetPosition(position);
  nd->SetOrientation(orientation);

  
  return nd;
}

void mitk::NavigationDataPlayer::StopPlaying()
{
  m_Playing = false;
  m_Pause = false;
  
  m_Stream = NULL;
  //TODO re init all data!! for playing again with different data
}

void mitk::NavigationDataPlayer::GetFirstData()
{    
  for (unsigned int index=0; index < m_NumberOfOutputs; index++)
  {
    switch(m_FileVersion)
    {
      case 1: 
        m_NextToPlayNavigationData[index] = ReadVersion1();
        //check if there is valid data in it

        if (m_NextToPlayNavigationData[index].IsNull())
        {
          StopPlaying();
          std::cout << "XML File is corrupt or has no NavigationData" << std::endl;
          return;
        }
        this->SetNthOutput(index, m_NextToPlayNavigationData[index]);

        m_StartTimeOfData[index] = m_NextToPlayNavigationData[index]->GetTimeStamp();
        break;
      default: //this case should not happen! therefore the return at this point
        return;
        break;
    }
  }
}

void mitk::NavigationDataPlayer::PauseContinuePlaying()
{
  //first case player runs and pause was called -> pause the player
  if(m_Playing && !m_Pause)
  {
    m_Playing == false;
    m_PauseTimeStamp = mitk::TimeStamp::GetInstance()->GetElapsed();
  }

  //second case player is in pause mode -> play at the last position
  if(!m_Playing && m_Pause)
  { 
    m_Playing = true;
    mitk::NavigationData::TimeStampType now = mitk::TimeStamp::GetInstance()->GetElapsed();
    m_StartPlayingTimeStamp = now + (m_PauseTimeStamp - m_StartPlayingTimeStamp);
  }
}
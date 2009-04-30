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
  m_NumberOfOutputs = 0;
  m_Pause = false;
  m_Playing = false;
  m_Stream = NULL;
  m_PlayerMode = NormalFile;
  m_FileName = "";
  m_FileVersion = 1;
  m_Playing = false;
  m_Pause = false;
  m_NumberOfOutputs = 0;
  m_StartPlayingTimeStamp = 0.0;
  m_PauseTimeStamp = 0.0;

  //To get a start time
  mitk::TimeStamp::GetInstance()->StartTracking(this);


}


mitk::NavigationDataPlayer::~NavigationDataPlayer()
{
  StopPlaying();
}


void mitk::NavigationDataPlayer::GenerateData()
{
  //Only produce new output if the player is started
  if (!m_Playing)
  {
    //The output is not valid anymore 
    for (unsigned int index = 0; index < m_NumberOfOutputs; index++)
    {
      mitk::NavigationData* output = this->GetOutput(index);
      assert(output);

      mitk::NavigationData::Pointer nd = mitk::NavigationData::New();
      mitk::NavigationData::PositionType position;
      mitk::NavigationData::OrientationType orientation(0.0,0.0,0.0,0.0);
      position.Fill(0.0);

      nd->SetPosition(position);
      nd->SetOrientation(orientation);
      nd->SetDataValid(false);

      output->Graft(nd);
    }
    return;
  }

  //first of all get current time
  TimeStampType now = mitk::TimeStamp::GetInstance()->GetElapsed();

  //now we make a little time arithmetic 
  //to get the elapsed time since the start of the player
  TimeStampType timeSinceStart = now - m_StartPlayingTimeStamp;
  
  //init the vectors
  std::vector< NavigationData::Pointer > nextCandidates; 
  std::vector< NavigationData::Pointer > lastCandidates; 
  std::vector< NavigationData::TimeStampType > currentTimeOfData;

  for (unsigned int index=0; index < m_NumberOfOutputs; index++)
  {
    nextCandidates.push_back(m_NextToPlayNavigationData.at(index));
    lastCandidates.push_back(m_NextToPlayNavigationData.at(index));
    
    currentTimeOfData.push_back(timeSinceStart + m_StartTimeOfData.at(index));
  }

  if (m_NextToPlayNavigationData.size() != m_NumberOfOutputs)
  {
    std::cout << "Mismatch in data" << std::endl;
    return;
  }

  // Now we try to find next NavigationData in the stream:
  // This means we step through the stream of NavigationDatas until we find 
  // a NavigationData which has a current timestamp (currentTimeOfData) greater 
  // then the current playing time. Then we store the data in 
  // m_NextToPlayNavigationData and take the last data (lastCandidates) for the 
  // output of this filter.
  //
  // The loop will stop when a suitable NavigationData is found or we reach EOF.
  // The timestamps of each recorded NavigationData should be equal 
  // therefore we take always the time from the first.
  while(nextCandidates[0]->GetTimeStamp() < currentTimeOfData[0])
  {
    for (unsigned int index=0; index < m_NumberOfOutputs; index++)
    {
      lastCandidates[index] = nextCandidates.at(index);

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
      for (unsigned int i = 0; i < m_NumberOfOutputs; i++)
      {
        if (nextCandidates.at(index).IsNull())
        {
          StopPlaying();
          return; //the case if no NavigationData is found, e.g. EOF, bad stream
        }
      }
    }

  }

  //Now lastCandidates stores the new output and nextCandidates is stored to the m_NextToPlay vector
  for (unsigned int index = 0; index < m_NumberOfOutputs; index++)
  {
    mitk::NavigationData* output = this->GetOutput(index);
    assert(output);

    output->Graft(lastCandidates.at(index));
    m_NextToPlayNavigationData[index] = nextCandidates.at(index);
  }
  
}


void mitk::NavigationDataPlayer::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}


void mitk::NavigationDataPlayer::InitPlayer()
{
  if (m_Stream == NULL)
  { 
    StopPlaying();
    std::cout << "Playing not possible. Wrong file name or path? " << std::endl;
    return;
  }
  if (!m_Stream->good())
  {
    StopPlaying();
    std::cout << "Playing not possible. Stream is not good!" << std::endl;
    return;
  }
  
  m_FileVersion = GetFileVersion(m_Stream); //first get the file version

  //check if we have a valid version
  if (m_FileVersion < 1)
  {
    StopPlaying();
    std::cout << "Playing not possible. Stream is not good!" << std::endl;
    return;
  }

  m_NumberOfOutputs = GetNumberOfNavigationDatas(m_Stream); //now read the number of Tracked Tools

  //with the information about the tracked tool number we can generate the output
  if (m_NumberOfOutputs > 0)
  {
    //Generate the output only if there are changes to the amount of outputs
    //This happens when the player is stopped and start again with different file
    if (this->GetNumberOfOutputs() != m_NumberOfOutputs)
    {    
      this->SetNumberOfOutputs(m_NumberOfOutputs);
    }

    GetFirstData(); //initialize the player with first data
  }
  else
  {
    std::cout << "The input stream seems to have NavigationData incompatible format" << std::endl;
    StopPlaying();
  }

}

unsigned int mitk::NavigationDataPlayer::GetFileVersion(std::istream* stream)
{
  if (stream==NULL)
  {
    std::cout << "No input stream set!" << std::endl;
    return 1;
  }
  if (!stream->good())
  {
    std::cout << "Stream not good!" << std::endl;
    return 1;
  }
  int version = 1;
  char str[255];
  stream->getline(str,255); //1st line this is the xml header

  //find out if this is a real xml file
  if (str[0] != '<' && str[1] != '?' && str[2] != 'x' && str[3] != 'm' && str[4] != 'l')
    return -1; 

  TiXmlElement* elem = new TiXmlElement("");
  *stream >> *elem; //2nd line this is the file version


  elem->QueryIntAttribute("Ver",&version);

  delete elem;
  
  if (version > 0)
    return version;
  else
    return 1;

}


unsigned int mitk::NavigationDataPlayer::GetNumberOfNavigationDatas(std::istream* stream)
{
  if (stream == NULL)
  {
    std::cout << "No input stream set!" << std::endl;
    return 0;
  }
  if (!stream->good())
  {
    std::cout << "Stream not good!" << std::endl;
    return 0;
  }

  //If something has changed in a future version of the XML definition e.g. navigationcount or addional parameters 
  //catch this here with a select case block (see GenerateData() method)

  int numberOfTools = 0;
  
  //if you stream to a TiXmlElement the whole Tag with its entire data is streamed to the element
  //therefore we only read one line and parse this
  char str[255];
  
  stream->getline(str,255); //read the line until end
  stream->getline(str,255); //3th line this one indicates additional parameters of the NavigationData object
  
  std::stringstream ss;
  ss << str;

  TiXmlElement* elem = new TiXmlElement("");
  ss >> *elem; 

  elem->QueryIntAttribute("ToolCount",&numberOfTools);

  delete elem;
  if (numberOfTools > 0)
    return numberOfTools;
  
  return 0;

}


mitk::NavigationData::Pointer mitk::NavigationDataPlayer::ReadVersion1()
{
  if (m_Stream == NULL)
  {
    m_Playing = false;
    std::cout << "Playing not possible. Wrong file name or path? " << std::endl;
    return NULL;
  }
  if (!m_Stream->good())
  {
    m_Playing = false;
    std::cout << "Playing not possible. Stream is not good!" << std::endl;
    return NULL;
  }

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

  TiXmlElement* elem = new TiXmlElement("");
  *m_Stream >> *elem;

  //check here if EOF (the query don't change the timestamp value which should always be > 0)
  elem->QueryDoubleAttribute("Time",&timestamp);
  if (timestamp == -1)
  {
    return NULL;  //the calling method should check the return value if it is valid/not NULL
  }

  elem->QueryFloatAttribute("X", &position[0]);
  elem->QueryFloatAttribute("Y", &position[1]);
  elem->QueryFloatAttribute("Z", &position[2]);

  elem->QueryFloatAttribute("QX", &orientation[0]);
  elem->QueryFloatAttribute("QY", &orientation[1]);
  elem->QueryFloatAttribute("QZ", &orientation[2]);
  elem->QueryFloatAttribute("QR", &orientation[3]);

  elem->QueryFloatAttribute("C00", &matrix[0][0]);
  elem->QueryFloatAttribute("C01", &matrix[0][1]);
  elem->QueryFloatAttribute("C02", &matrix[0][2]);
  elem->QueryFloatAttribute("C03", &matrix[0][3]);
  elem->QueryFloatAttribute("C04", &matrix[0][4]);
  elem->QueryFloatAttribute("C05", &matrix[0][5]);
  elem->QueryFloatAttribute("C10", &matrix[1][0]);
  elem->QueryFloatAttribute("C11", &matrix[1][1]);
  elem->QueryFloatAttribute("C12", &matrix[1][2]);
  elem->QueryFloatAttribute("C13", &matrix[1][3]);
  elem->QueryFloatAttribute("C14", &matrix[1][4]);
  elem->QueryFloatAttribute("C15", &matrix[1][5]);

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

  nd->SetTimeStamp(timestamp);
  nd->SetPosition(position);
  nd->SetOrientation(orientation);
  nd->SetCovErrorMatrix(matrix);
  nd->SetDataValid(dataValid);
  nd->SetHasOrientation(hasOrientation);
  nd->SetHasPosition(hasPosition);

  delete elem;
  return nd;
}


void mitk::NavigationDataPlayer::StartPlaying()
{
  if (m_Stream == NULL)
  {   
    m_Playing = false;

    //Perhaps the SetStream method was not called so we do this when a FileName is set with SetStream(PlayerMode)
    if (m_FileName != "")
    {
      //The PlayerMode is initialized with LastSetStream 
      //SetStream also calls InitPlayer()
      SetStream(m_PlayerMode);
    }

    //now check again
    if (m_Stream == NULL)
    {       
      StopPlaying();
      std::cout << "Playing not possible. Wrong file name or path? " << std::endl;
      return;
    }
  }

  if (!m_Playing && m_Stream->good())
  {
    m_Playing = true; //starts the player
    m_StartPlayingTimeStamp = mitk::TimeStamp::GetInstance()->GetElapsed();
  }
  else
  {
    std::cout << "Player already started or stream is not good" << std::endl;
    StopPlaying();
  }

}


void mitk::NavigationDataPlayer::StopPlaying()
{
  //re init all data!! for playing again with different data
  //only PlayerMode and FileName are not changed
  m_Pause = false;
  m_Playing = false;
  m_Stream = NULL;
  m_FileVersion = 1;
  m_Playing = false;
  m_Pause = false;
  m_StartPlayingTimeStamp = 0.0;
  m_PauseTimeStamp = 0.0;
  m_NextToPlayNavigationData.clear();
  m_StartTimeOfData.clear();
}


void mitk::NavigationDataPlayer::GetFirstData()
{ 

  //Here we read the first lines of input (dependend on the number of inputs)
  for (unsigned int index=0; index < m_NumberOfOutputs; index++)
  {
    //Here we init the vector for later use
    m_NextToPlayNavigationData.push_back(NULL);
    m_StartTimeOfData.push_back(0.0);
    mitk::NavigationData::Pointer nd = this->GetOutput(index);

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

        //Have a look it the output was set already without this check the pipline will disconnect after a start/stop cycle
        if (nd.IsNull())
          this->SetNthOutput(index, m_NextToPlayNavigationData[index]);

        m_StartTimeOfData[index] = m_NextToPlayNavigationData[index]->GetTimeStamp();
        break;
      default: //this case should not happen! therefore the return at this point
        return;
        break;
    }
  }
}


void mitk::NavigationDataPlayer::Pause()
{
  //player runs and pause was called -> pause the player
  if(m_Playing && !m_Pause)
  {
    m_Playing = false;
    m_Pause = true;
    m_PauseTimeStamp = mitk::TimeStamp::GetInstance()->GetElapsed();
  }
  else
  {
    std::cout << "Player is either not started or already in paused" << std::endl;
  }

}


void mitk::NavigationDataPlayer::Resume()
{
  //player is in pause mode -> play at the last position
  if(!m_Playing && m_Pause)
  { 
    m_Playing = true;
    mitk::NavigationData::TimeStampType now = mitk::TimeStamp::GetInstance()->GetElapsed();
    m_StartPlayingTimeStamp = now + (m_PauseTimeStamp - m_StartPlayingTimeStamp);
  }
  else
  {
    std::cout << "Player is not paused!" << std::endl;
  }
}


void mitk::NavigationDataPlayer::SetStream( PlayerMode mode )
{
  m_Stream = NULL;

  if (!itksys::SystemTools::FileExists(m_FileName.c_str()))
  {
    std::cout << "File dont exist!" << std::endl;
    return;
  }
  switch(m_PlayerMode)
  {
  case NormalFile:

    m_Stream = new std::ifstream(m_FileName.c_str());
    
    break;
  case ZipFile:

    m_Stream = NULL;
    std::cout << "Sorry no ZipFile support yet";

    break;
  default:
    m_Stream = NULL;
    break;
  }

  this->Modified();
  InitPlayer();
}


void mitk::NavigationDataPlayer::SetStream( std::istream* stream )
{
  if (!stream->good())
  {
    std::cout << "The stream is not good" << std::endl;
    return;
  }

  m_Stream = stream;

  this->Modified();
  InitPlayer();
}


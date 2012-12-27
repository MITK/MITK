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

#include "mitkNavigationDataPlayer.h"

#include <itksys/SystemTools.hxx>
#include <mitkTimeStamp.h>
#include <fstream>

//includes for exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"

mitk::NavigationDataPlayer::NavigationDataPlayer() : mitk::NavigationDataPlayerBase()
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
  m_parentElement = NULL;
  m_currentNode = NULL;
  m_StreamEnd = false;
  m_StreamSetOutsideFromClass = false;

  //To get a start time
  mitk::TimeStamp::GetInstance()->Start(this);
}


mitk::NavigationDataPlayer::~NavigationDataPlayer()
{
  StopPlaying();
  delete m_parentElement;
}


void mitk::NavigationDataPlayer::GenerateData()
{
  //Only produce new output if the player is started
  if (!m_Playing) //m_Playing==true means player has started
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
    MITK_ERROR << "Mismatch in data";
    return;
  }

  // Now we try to find next NavigationData in the stream:
  // This means we step through the stream of NavigationDatas until we find
  // a NavigationData which has a current timestamp (currentTimeOfData) greater
  // than the current playing time. Then we store the data in
  // m_NextToPlayNavigationData and take the last data (lastCandidates) for the
  // output of this filter.
  //
  // The loop will stop when a suitable NavigationData is found or we reach EOF.
  // The timestamps of each recorded NavigationData should be equal
  // therefore we take always the time from the first.


  while( nextCandidates[0]->GetTimeStamp() < currentTimeOfData[0])
  {
    for (unsigned int index=0; index < m_NumberOfOutputs; index++)
    {
      lastCandidates[index] = nextCandidates.at(index);
      switch(m_FileVersion) // m_FileVersion indicates which XML encoding is used
      {
      case 1:
        nextCandidates[index] = ReadVersion1();
        break;
      default: //this case should not happen! therefore return at this point
        MITK_ERROR << "File encoding format was not stored, aborting!";
        return;
        break;
      }

      //check if the input stream delivered a correct NavigationData object
      for (unsigned int i = 0; i < m_NumberOfOutputs; i++)
      {
        if (nextCandidates.at(index).IsNull())
        //Stops playing because there is now more nextCandidate, the file ended for all outputs
        {
          m_StreamEnd = true;
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
    StreamInvalid("Playing not possible. Wrong file name or path?");
    return;
  }

  if (!m_Stream->good())
  {
    StreamInvalid("Playing not possible. Stream is not good!");
    return;
  }

  //first get the file version
  m_FileVersion = GetFileVersion(m_Stream);

  //check if we have a valid version: m_FileVersion has to be always bigger than 1 for playing
  if (m_FileVersion < 1)
  {
    StreamInvalid("Playing not possible. Invalid file version!");
    return;
  }

  if(m_NumberOfOutputs == 0) {m_NumberOfOutputs = GetNumberOfNavigationDatas(m_Stream);}

  //with the information about the tracked tool number we can generate the output
  if (m_NumberOfOutputs > 0)
  {
    //Generate the output only if there are changes to the amount of outputs
    //This happens when the player is stopped and start again with different file
    if (this->GetNumberOfOutputs() != m_NumberOfOutputs) {SetNumberOfOutputs(m_NumberOfOutputs);}
    //initialize the player with first data
    GetFirstData();
    //set stream valid
    m_ErrorMessage = "";
    m_StreamValid = true;
  }
  else
  {
    StreamInvalid("The input stream seems to have NavigationData incompatible format");
    return;
  }

}

unsigned int mitk::NavigationDataPlayer::GetFileVersion(std::istream* stream)
{
  if (stream==NULL)
  {
    MITK_ERROR << "No input stream set!";
    mitkThrowException(mitk::IGTException)<<"No input stream set!";
  }
  if (!stream->good())
  {
    MITK_ERROR << "Stream is not good!";
    mitkThrowException(mitk::IGTException)<<"Stream is not good!";
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

  if (version > 0)
    return version;
  else
    return 0;

}


unsigned int mitk::NavigationDataPlayer::GetNumberOfNavigationDatas(std::istream* stream)
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

  if (numberOfTools > 0)
    return numberOfTools;

  return 0;

}


mitk::NavigationData::Pointer mitk::NavigationDataPlayer::ReadVersion1()
{
  if (m_Stream == NULL)
  {
    m_Playing = false;
    MITK_ERROR << "Playing not possible. Wrong file name or path? ";
    mitkThrowException(mitk::IGTException) << "Playing not possible. Wrong file name or path? ";
  }
  if (!m_Stream->good())
  {
    m_Playing = false;
    MITK_ERROR << "Playing not possible. Stream is not good!";
    mitkThrowException(mitk::IGTException) << "Playing not possible. Stream is not good!";
  }

  /*TiXmlElement* elem = new TiXmlElement("");
  m_currentNode = m_parentElement->IterateChildren(m_currentNode);

  if(m_currentNode)
  {
      elem = m_currentNode->ToElement();
  }*/

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

  if(delElem)
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
    //CreateStreamFromFilename also calls InitPlayer()
    try
      {
      CreateStreamFromFilename();
      }
    catch(mitk::IGTIOException e)
      {
      MITK_ERROR << "Cannot create stream from filename, please check the stream";
      throw e; //TODO replace by macro
      }
    catch(mitk::IGTException e2)
      {
      MITK_ERROR << "Cannot open the file, please check the file";
      throw e2; //TODO replace by macro
      }
    }

    //now check again
    if (m_Stream == NULL)
    {
      StopPlaying();
      MITK_ERROR << "Playing not possible. Wrong file name or path?";
      mitkThrowException(mitk::IGTException) << "Playing not possible. Wrong file name or path?";
    }
  }

  if (!m_Playing && m_Stream->good())
  {
    m_Playing = true;
    m_StartPlayingTimeStamp = mitk::TimeStamp::GetInstance()->GetElapsed();
  }
  else
  {
    MITK_ERROR << "Player already started or stream is not good!";
    StopPlaying();
  }

}




void mitk::NavigationDataPlayer::StopPlaying()
{
  //re init all data!! for playing again with different data
  //only PlayerMode and FileName are not changed
  m_Pause = false;
  m_Playing = false;
  if (!m_StreamSetOutsideFromClass)
    {delete m_Stream;}
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
          m_StreamEnd = true;
          StopPlaying();
          mitkThrowException(mitk::IGTIOException) << "XML File is corrupt or has no NavigationData.";
        }

        //Have a look it the output was set already without this check the pipline will disconnect after a start/stop cycle
        if (nd.IsNull()) {this->SetNthOutput(index, m_NextToPlayNavigationData[index]);}

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
    MITK_ERROR << "Player is either not started or already is paused" << std::endl;
  }

}


void mitk::NavigationDataPlayer::Resume()
{
  //player is in pause mode -> play at the last position
  if(!m_Playing && m_Pause)
  {
    m_Playing = true;
    m_Pause = false;
    mitk::NavigationData::TimeStampType now = mitk::TimeStamp::GetInstance()->GetElapsed();

    // in this case m_StartPlayingTimeStamp is set to the total elapsed time with NO playback
    m_StartPlayingTimeStamp = now - (m_PauseTimeStamp - m_StartPlayingTimeStamp);
  }
  else
  {
    MITK_ERROR << "Player is not paused!" << std::endl;
  }
}


void mitk::NavigationDataPlayer::CreateStreamFromFilename()
{
  m_Stream = NULL;

  if (!itksys::SystemTools::FileExists(m_FileName.c_str()))
  {
    mitkThrowException(mitk::IGTIOException) << "File does not exist!";
  }

  switch(m_PlayerMode)
  {
  case NormalFile:
    m_Stream = new std::ifstream(m_FileName.c_str());
    m_StreamSetOutsideFromClass = false;
    break;

  case ZipFile:
    m_Stream = NULL;
    MITK_ERROR << "Sorry no ZipFile support yet";
    break;

  default:
    m_Stream = NULL;
    mitkThrowException(mitk::IGTException) << "The stream is NULL";
    break;
  }

  this->Modified();
  InitPlayer();
}


void mitk::NavigationDataPlayer::SetStream( std::istream* stream )
{
  if ( (stream == NULL) || (!stream->good()))
  {
    // throw an exception for stream=NULL or it is not good
    mitkThrowException(mitk::IGTException) << "The stream is NULL or it is not good";
    m_StreamEnd = true;
    return;
  }

  m_Stream = stream;
  m_StreamSetOutsideFromClass = true;

  this->Modified();
  InitPlayer();
}

bool mitk::NavigationDataPlayer::IsAtEnd()
{
  return this->m_StreamEnd;
}

void mitk::NavigationDataPlayer::StreamInvalid(std::string message)
{
  m_StreamEnd = true;
  StopPlaying();
  m_ErrorMessage = message;
  m_StreamValid = false;
  mitkThrowException(mitk::IGTIOException) << "Invalid stream!";
}
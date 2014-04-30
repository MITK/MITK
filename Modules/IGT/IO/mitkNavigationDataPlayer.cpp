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
#include <mitkIGTTimeStamp.h>
#include <fstream>
#include "mitkNavigationDataReaderXML.h"

#include "mitkIGTException.h"

mitk::NavigationDataPlayer::NavigationDataPlayer()
  : m_CurPlayerState(PlayerStopped),
  m_StartPlayingTimeStamp(0.0), m_PauseTimeStamp(0.0)
{
  // to get a start time
  mitk::IGTTimeStamp::GetInstance()->Start(this);
}

mitk::NavigationDataPlayer::~NavigationDataPlayer()
{
  StopPlaying();
}

void mitk::NavigationDataPlayer::GenerateData()
{
  if ( m_NavigationDataSet->Size() == 0 )
  {
    MITK_WARN << "Cannot do anything with empty set of navigation datas.";
    return;
  }

  //Only produce new output if the player is started
  if (m_CurPlayerState != PlayerRunning)
  {
    //The output is not valid anymore
    this->GraftEmptyOutput();
    return;
  }

  // get elapsed time since start of playing
  m_TimeStampSinceStart = mitk::IGTTimeStamp::GetInstance()->GetElapsed() - m_StartPlayingTimeStamp;

  // add offset of the first navigation data to the timestamp to start playing
  // imediatly with the first navigation data (not to wait till the first time
  // stamp is reached)
  TimeStampType timeStampSinceStartWithOffset = m_TimeStampSinceStart
      + m_NavigationDataSet->Begin()->at(0)->GetIGTTimeStamp();

  // iterate through all NavigationData objects of the given tool index
  // till the timestamp of the NavigationData is greater then the given timestamp
  for (; m_NavigationDataSetIterator != m_NavigationDataSet->End(); ++m_NavigationDataSetIterator)
  {
    // test if the timestamp of the successor is greater than the time stamp
    if ( m_NavigationDataSetIterator+1 == m_NavigationDataSet->End() ||
        (m_NavigationDataSetIterator+1)->at(0)->GetIGTTimeStamp() > timeStampSinceStartWithOffset )
    {
      break;
    }
  }

  for (unsigned int index = 0; index < GetNumberOfOutputs(); index++)
  {
    mitk::NavigationData* output = this->GetOutput(index);
    if( !output ) { mitkThrowException(mitk::IGTException) << "Output of index "<<index<<" is null."; }

    output->Graft(m_NavigationDataSetIterator->at(index));
  }

  // stop playing if the last NavigationData objects were grafted
  if (m_NavigationDataSetIterator+1 == m_NavigationDataSet->End())
  {
    this->StopPlaying();

    // start playing again if repeat is enabled
    if ( m_Repeat ) { this->StartPlaying(); }
  }
}

void mitk::NavigationDataPlayer::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}

void mitk::NavigationDataPlayer::StartPlaying()
{
  // make sure that player is initialized before playing starts
  this->InitPlayer();

  // set state and iterator for playing from start
  m_CurPlayerState = PlayerRunning;
  m_NavigationDataSetIterator = m_NavigationDataSet->Begin();

  // reset playing timestamps
  m_PauseTimeStamp = 0;
  m_TimeStampSinceStart = 0;

  // timestamp for indicating playing start set to current elapsed time
  m_StartPlayingTimeStamp = mitk::IGTTimeStamp::GetInstance()->GetElapsed();
}

void mitk::NavigationDataPlayer::StopPlaying()
{
  m_CurPlayerState = PlayerStopped;
}

void mitk::NavigationDataPlayer::Pause()
{
  //player runs and pause was called -> pause the player
  if(m_CurPlayerState == PlayerRunning)
  {
    m_CurPlayerState = PlayerPaused;
    m_PauseTimeStamp = mitk::IGTTimeStamp::GetInstance()->GetElapsed();
  }
  else
  {
    MITK_ERROR << "Player is either not started or already is paused" << std::endl;
  }
}

void mitk::NavigationDataPlayer::Resume()
{
  // player is in pause mode -> play at the last position
  if(m_CurPlayerState == PlayerPaused)
  {
    m_CurPlayerState = PlayerRunning;

    // in this case m_StartPlayingTimeStamp is set to the total elapsed time with NO playback
    m_StartPlayingTimeStamp = mitk::IGTTimeStamp::GetInstance()->GetElapsed()
      - (m_PauseTimeStamp - m_StartPlayingTimeStamp);
  }
  else
  {
    MITK_ERROR << "Player is not paused!" << std::endl;
  }
}

mitk::NavigationDataPlayer::PlayerState mitk::NavigationDataPlayer::GetCurrentPlayerState()
{
  return m_CurPlayerState;
}

mitk::NavigationDataPlayer::TimeStampType mitk::NavigationDataPlayer::GetTimeStampSinceStart()
{
  return m_TimeStampSinceStart;
}

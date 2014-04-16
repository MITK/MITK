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
  //Only produce new output if the player is started
  if (m_CurPlayerState != PlayerRunning)
  {
    //The output is not valid anymore
    this->GraftEmptyOutput();
    return;
  }

  // get elapsed time since start of playing
  m_TimeStampSinceStart = mitk::IGTTimeStamp::GetInstance()->GetElapsed() - m_StartPlayingTimeStamp;


  // iterate through all NavigationData objects of the given tool index
  // till the timestamp of the NavigationData is greater then the given timestamp
  for (; m_NavigationDataSetIterator != m_NavigationDataSet->End(); ++m_NavigationDataSetIterator)
  {
    if ( m_NavigationDataSetIterator->at(0)->GetIGTTimeStamp() > m_TimeStampSinceStart)
    {
      break;
    }
  }

  // first element was greater than timestamp -> return null
  if ( m_NavigationDataSetIterator == m_NavigationDataSet->Begin() )
  {
    MITK_WARN("NavigationDataSet") << "No NavigationData was recorded before given timestamp.";

    //The output is not at this time
    this->GraftEmptyOutput();
    return;
  }

  for (unsigned int index = 0; index < m_NumberOfOutputs; index++)
  {
    mitk::NavigationData* output = this->GetOutput(index);
    if( !output ) { mitkThrowException(mitk::IGTException) << "Output of index "<<index<<" is null."; }

    mitk::NavigationDataSet::NavigationDataSetIterator curIterator = m_NavigationDataSetIterator-1;
    output->Graft(curIterator->at(index));
  }

  // stop playing if the last NavigationData objects were grafted
  if (m_NavigationDataSetIterator == m_NavigationDataSet->End())
  {
    this->StopPlaying();
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

  // timestamp for indicating playing start is set to the past
  // so that the first navigation data object will be shown NOW
  m_StartPlayingTimeStamp = mitk::IGTTimeStamp::GetInstance()->GetElapsed()
      - m_NavigationDataSet->Begin()->at(0)->GetIGTTimeStamp();
}

void mitk::NavigationDataPlayer::StopPlaying()
{
  m_CurPlayerState = PlayerStopped;

  // reset playing timestamps
  m_StartPlayingTimeStamp = 0;
  m_PauseTimeStamp = 0;
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

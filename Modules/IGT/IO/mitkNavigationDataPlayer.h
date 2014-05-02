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

#ifndef MITKNavigationDataPlayer_H_HEADER_INCLUDED_
#define MITKNavigationDataPlayer_H_HEADER_INCLUDED_

#include <mitkNavigationDataPlayerBase.h>

#include <itkMultiThreader.h>

namespace mitk {
  /**Documentation
  * \brief This class is used to play recorded (see mitkNavigationDataRecorder class) NavigationDataSets.
  *
  *  TODO
  *
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataPlayer : public NavigationDataPlayerBase
  {
  public:
    mitkClassMacro(NavigationDataPlayer, NavigationDataPlayerBase);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

      enum PlayerState { PlayerStopped, PlayerRunning, PlayerPaused };
    typedef mitk::NavigationData::TimeStampType TimeStampType;

    /**
    * \brief Used for pipeline update just to tell the pipeline that we always have to update
    */
    virtual void UpdateOutputInformation();

    /**
    * \brief This method starts the player.
    *
    * The method mitk::NavigationDataPlayer::SetNavigationDataSet() has to be called before.
    *
    * @throw mitk::IGTException If m_NavigationDataSet is null.
    */
    void StartPlaying();

    /**
    * \brief Stops the player and closes the stream.
    * After a call of StopPlaying(), StartPlaying() must be called to get new
    * output data.
    */
    void StopPlaying();

    /**
    * \brief This method pauses the player. If you want to play again call Resume()
    */
    void Pause();

    /**
    * \brief This method resumes the player when it was paused.
    */
    void Resume();

    PlayerState GetCurrentPlayerState();

    TimeStampType GetTimeStampSinceStart();

  protected:
    NavigationDataPlayer();
    virtual ~NavigationDataPlayer();

    /**
    * \brief Set outputs to the navigation data object corresponding to current time.
    */
    virtual void GenerateData();

    PlayerState m_CurPlayerState;

    /**
    * \brief The start time of the playing. Set in the method mitk::NavigationDataPlayer::StartPlaying().
    */
    TimeStampType m_StartPlayingTimeStamp;

    /**
    * \brief Stores the time when a pause began.
    */
    TimeStampType m_PauseTimeStamp;

    TimeStampType m_TimeStampSinceStart;
  };
} // namespace mitk

#endif /* MITKNavigationDataPlayer_H_HEADER_INCLUDED_ */

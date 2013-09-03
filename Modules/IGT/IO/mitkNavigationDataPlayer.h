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
#include <mitkNavigationDataSource.h>
#include <mitkNavigationDataRecorder.h> //for the Recording Mode enum
#include "mitkTrackingDevice.h"

#include <itkMultiThreader.h>

#include <istream>


namespace mitk {
  /**Documentation
  * \brief This class is used to play recorded (see mitkNavigationDataRecorder class) files.
  *
  * If you want to play a file you have to set an input stream. This can be an own one (use StartPlaying(std::istream*))
  * or a preset (use StartPlaying()). The presets are NormalFile and ZipFile and can be set with the method
  * SetPlayerMode(PlayerMode). The presets need a FileName. Therefore the FileName must be set before the preset.
  * For pausing the player call Pause(). A call of Resume() will continue the playing.
  *
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataPlayer : public NavigationDataPlayerBase
  {
  public:
    mitkClassMacro(NavigationDataPlayer, NavigationDataPlayerBase);
    itkNewMacro(Self);

    /**
    * \brief sets the file name and path for the PlayerMode NormalFile and ZipFile
    */
    itkSetStringMacro(FileName);

    /**
    * \brief returns the file name and path for the PlayerMode NormalFile and ZipFile
    */
    itkGetStringMacro(FileName);


    /**
    * \brief Used for pipeline update just to tell the pipeline that we always have to update
    */
    virtual void UpdateOutputInformation();

    /**
     * \brief This method starts the player.
     *
     * Before the stream has to be set. Either with a PlayingMode (SetStream(PlayerMode)) and FileName. Or
     * with an own inputstream (SetStream(istream*)).
     *
     * @throw mitk::IGTIOException Throws an exception if the file cannot be opened.
     * @throw mitk::IGTIOException Throws an exception if there is no valid filename.
     * @throw mitk::IGTIOException Throws an exception if the file is damaged.
     * @throw mitk::IGTException Throws an exception if there is no stream (i.e stream=NULL).
     */
    void StartPlaying();

    /**
     * \brief Stops the player and closes the stream. After a call of StopPlaying()
     * StartPlaying() must be called to get new output data
     *
     * \warning the output is generated in this method because we know first about the number of output after
     * reading the first lines of the XML file. Therefore you should assign your output after the call of this method
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

    /**
     * \brief This method checks if player arrived at end of file.
     *
     */
    bool IsAtEnd();

    /**
     * \brief Sets the stream of this player.
     * @throw mitk::IGTException Throws an exception if stream is NULL or if it is not good.
     * \deprecated Will be removed in one of the next releases. Use SetFileName() instead.
     */
    void SetStream(std::istream* stream);

  protected:
    enum PlayerState { PlayerStopped, PlayerRunning, PlayerPaused };

    PlayerState m_CurPlayerState;

    NavigationDataPlayer();
    virtual ~NavigationDataPlayer();

    typedef mitk::NavigationData::TimeStampType TimeStampType;

    /**
     * \brief filter execute method
     */
    virtual void GenerateData();

    /**
     * \brief This method initializes the player with first data
     */
    void InitPlayer();

    mitk::NavigationDataSet::NavigationDataSetIterator m_NavigationDataSetIterator;

    std::string m_FileName; ///< stores the filename

    unsigned int m_NumberOfOutputs; ///< stores the number of outputs known from the XML document

    TimeStampType m_StartPlayingTimeStamp; ///< the starttime of the playing set in the method StartPlaying()
    TimeStampType m_PauseTimeStamp; ///< stores the beginning of a pause

    std::vector<TimeStampType> m_StartTimeOfData; ///< stores the start time of the different tools
  };
} // namespace mitk

#endif /* MITKNavigationDataPlayer_H_HEADER_INCLUDED_ */

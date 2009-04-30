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


#ifndef MITKNavigationDataPlayer_H_HEADER_INCLUDED_
#define MITKNavigationDataPlayer_H_HEADER_INCLUDED_

#include <mitkNavigationDataSource.h>
#include <mitkNavigationDataRecorder.h> //for the Recording Mode enum
#include "mitkTrackingDevice.h"

#include <itkMultiThreader.h>

#include "tinyxml.h"
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
  class MITK_IGT_EXPORT NavigationDataPlayer : public NavigationDataSource
  {
  public:
    mitkClassMacro(NavigationDataPlayer, NavigationDataSource);
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
    * 
    *\warning This method is not tested yet. It is not save to use!
    */
    void Pause();

    /**
    * \brief This method resumes the player when it was paused. 
    * 
    *\warning This method is not tested yet. It is not save to use!
    */
    void Resume();

    /**
    * \brief The PlayerMode is used for generating a presetted output stream. You do not need to
    * set it if you want to use your own stream.
    *
    * There are:
    * NormalFile: ifstream
    * ZipFile: not implemented yet
    *
    *\warning The ZipFile Mode is not implemented yet
    */
    enum PlayerMode
    {
      NormalFile,
      ZipFile
    };

    /**
    * \brief sets the recording mode which causes different types of output streams
    * This method is overloaded with SetStream( ostream* )
    */
    void SetStream(PlayerMode mode);

    /**
    * \brief sets the recording mode which causes different types of output streams
    * This method is overloaded with SetStream( PlayerMode )
    */
    void SetStream(std::istream* stream);

  protected:
    NavigationDataPlayer();
    virtual ~NavigationDataPlayer();

    typedef mitk::NavigationData::TimeStampType TimeStampType;

    /**
    * \brief filter execute method 
    */
    virtual void GenerateData();

    /**
    * \brief Returns the file version out of the XML document. 
    */
    unsigned int GetFileVersion(std::istream* stream);

    /**
    * \brief Returns the number of tracked tools out of the XML document.
    */
    unsigned int GetNumberOfNavigationDatas(std::istream* stream);

    /**
    * \brief Gets the first data for initializing the player
    */
    void GetFirstData();

    /**
    * \brief This method reads one line of the XML document and returns the data as a NavigationData object
    * If there is a new file version another method must be added which reads this data.
    */
    mitk::NavigationData::Pointer ReadVersion1();

    /**
    * \brief This method initializes the player with first data  
    */
    void InitPlayer();

    std::istream* m_Stream; ///< stores a pointer to the input stream

    PlayerMode m_PlayerMode; ///< stores the mode for the presetted PlayerMode sieh enum PlayerMode

    std::string m_FileName; ///< stores the filename

    unsigned int m_FileVersion; ///< indicates which XML encoding is used

    bool m_Playing; ///< indicates whether the generateoutput method generates new output or not

    bool m_Pause; ///< indicates if the player is paused

    unsigned int m_NumberOfOutputs; ///< stores the number of outputs known from the XML document

    TimeStampType m_StartPlayingTimeStamp; ///< the starttime of the playing set in the method StartPlaying()

    TimeStampType m_PauseTimeStamp; ///< stores the beginning of a pause

    std::vector<NavigationData::Pointer> m_NextToPlayNavigationData; ///< stores the next possible candidate for playing

    std::vector<TimeStampType> m_StartTimeOfData; ///< stores the start time of the different tools
  };
} // namespace mitk

#endif /* MITKNavigationDataPlayer_H_HEADER_INCLUDED_ */
